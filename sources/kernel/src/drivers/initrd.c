#include <klibc/assert.h>
#include <klibc/archive/ustar.h>
#include <klibc/mem/bytes.h>

#include <kernel/drivers/initrd.h>
#include <kernel/vfs/node-ops.h>
#include <kernel/vfs/file-ops.h>
#include <kernel/mem/heap.h>


// MARK: - details

typedef struct node_fs_data node_fs_data;
struct node_fs_data {
  initrd             *dev;
  str                 abspath;
  const ustar_header *header;
};


static void initrd__release_node(vfs_node *node) {
  allocator a = heap_allocator();

  node_fs_data *fs_data = node->fs_data;
  assert(fs_data != NULL);

  str_free  (a, &fs_data->abspath);
  deallocate(a, fs_data, sizeof(node_fs_data));
  deallocate(a, node,    sizeof(vfs_node));
}


// MARK: - ops decl
static RESULT(vfs_node_ref,  vfs_err) initrd__lookup  (vfs_node *self, str name);
static RESULT(str,           vfs_err) initrd__readlink(vfs_node *self, allocator a);
static RESULT(vfs_file_ref,  vfs_err) initrd__open    (vfs_node *self, u32 flags);
static RESULT(vfs_stat_desc, vfs_err) initrd__stat    (vfs_node *self);

static const vfs_node_ops initrd_node_ops = {
  .lookup   = initrd__lookup,
  .readlink = initrd__readlink,
  .open     = initrd__open,
  .stat     = initrd__stat,
};


static RESULT(UNIT,           vfs_err) initrd__close  (vfs_file *self);
static RESULT(usize,          vfs_err) initrd__read   (vfs_file *self, span buf);
static RESULT(u64,            vfs_err) initrd__seek   (vfs_file *self, i64 offset, vfs_seek_whence whence);
static RESULT(vfs_dirent_ptr, vfs_err) initrd__readdir(vfs_file *self, allocator a);

static const vfs_file_ops initrd_file_ops = {
  .close   = initrd__close,
  .read    = initrd__read,
  .seek    = initrd__seek,
  .readdir = initrd__readdir,
};


// MARK: - node ops

static RESULT(vfs_node_ref, vfs_err) initrd__lookup(vfs_node *self, str name) {
  assert(self != NULL);

  allocator a = heap_allocator();

  node_fs_data *fs_data = self->fs_data;
  assert(fs_data != NULL);

  str abspath = str_format(a, "%s/%s", fs_data->abspath, name);

  const ustar_header *hdr = ustar_lookup(fs_data->dev->buffer, abspath);
  if (hdr == NULL) {
    str_free(a, &abspath);
    return (RESULT(vfs_node_ref, vfs_err)) ERR(VFS_ENOTFOUND);
  }

  vfs_node     *new_node    = allocate(a, sizeof(vfs_node));
  node_fs_data *new_fs_data = allocate(a, sizeof(node_fs_data));

  new_fs_data->dev     = fs_data->dev;
  new_fs_data->abspath = abspath;
  new_fs_data->header  = hdr;

  new_node->ops     = &initrd_node_ops;
  new_node->release = initrd__release_node;
  new_node->fs_data = new_fs_data;

  switch (ustar_entry_type(hdr)) {
    case USTAR_FILETYPE_NORMAL:
    case USTAR_FILETYPE_HARDLINK:
    case USTAR_FILETYPE_PIPE:
      new_node->type = VFS_NODETYPE_FILE;
      break;

    case USTAR_FILETYPE_DIRECTORY:
      new_node->type = VFS_NODETYPE_DIRECTORY;
      break;

    case USTAR_FILETYPE_SYMLINK:
      new_node->type = VFS_NODETYPE_SYMLINK;
      break;

    case USTAR_FILETYPE_CHARDEV:
    case USTAR_FILETYPE_BLOCKDEV:
      new_node->type = VFS_NODETYPE_DEVICE;
      break;
  }

  return (RESULT(vfs_node_ref, vfs_err)) OK(vfs_node_incref(new_node));
}


static RESULT(str, vfs_err) initrd__readlink(vfs_node *self, allocator a) {
  assert(self != NULL);
  assert(self->type == VFS_NODETYPE_SYMLINK);

  node_fs_data *fs_data = self->fs_data;
  assert(fs_data != NULL);

  str link = ustar_entry_linkname(fs_data->header, a);

  return (RESULT(str, vfs_err)) OK(link);
}


static RESULT(vfs_file_ref, vfs_err) initrd__open(vfs_node *self, u32 flags) {
  assert(self != NULL);

  allocator a = heap_allocator();

  vfs_file *f = allocate(a, sizeof(vfs_file));
  f->ops      = &initrd_file_ops;
  f->node     = vfs_node_incref(self);
  f->flags    = flags;

  return (RESULT(vfs_file_ref, vfs_err)) OK(f);
}


static RESULT(vfs_stat_desc, vfs_err) initrd__stat(vfs_node *self) {
  assert(self != NULL);

  node_fs_data *fs_data = self->fs_data;
  assert(fs_data != NULL);

  vfs_stat_desc stat = {
    .type  = self->type,
    .perms = {
      .uid  = ustar_entry_uid (fs_data->header),
      .gid  = ustar_entry_gid (fs_data->header),
      .mode = ustar_entry_mode(fs_data->header),
    },
    .size  = ustar_entry_size (fs_data->header),
    .mtime = ustar_entry_mtime(fs_data->header),
  };

  return (RESULT(vfs_stat_desc, vfs_err)) OK(stat);
}


// MARK: - file ops

static RESULT(UNIT, vfs_err) initrd__close(vfs_file *self) {
  assert(self != NULL);

  vfs_node_decref(self->node);
  deallocate(heap_allocator(), self, sizeof(vfs_file));

  return (RESULT(UNIT, vfs_err)) OK({});
}


static RESULT(usize, vfs_err) initrd__read(vfs_file *self, span buf) {
  assert(self != NULL);
  assert(buf.data != NULL && buf.size > 0);

  node_fs_data *fs_data = self->node->fs_data;
  assert(fs_data != NULL);

  usize filesize        = ustar_entry_size(fs_data->header);
  usize bytes_available = filesize - (usize)self->offset;
  usize bytes_read      = (buf.size > bytes_available ? bytes_available : buf.size);

  if (bytes_read > 0) {
    const_span content = ustar_entry_data(fs_data->header);
    memcpy(buf.data, (const void *)((uptr)content.data + (usize)self->offset), bytes_read);

    self->offset += bytes_read;
  }

  return (RESULT(usize, vfs_err)) OK(bytes_read);
}


static RESULT(u64, vfs_err) initrd__seek(vfs_file *self, i64 offset, vfs_seek_whence whence) {
  assert(self != NULL);

  node_fs_data *fs_data = self->node->fs_data;
  assert(fs_data != NULL);

  u64 filesize = ustar_entry_size(fs_data->header);

  switch (whence) {
    case VFS_SEEK_SET:
      if (offset < 0 || (u64)offset > filesize) {
        return (RESULT(u64, vfs_err)) ERR(VFS_EINVAL);
      }

      self->offset = (u64)offset;
      break;

    case VFS_SEEK_CUR:
      if (offset < 0) {
        u64 mag = (u64)(-(offset + 1)) + 1;

        if (self->offset < mag) {
          return (RESULT(u64, vfs_err)) ERR(VFS_EINVAL);
        }

        self->offset -= mag;
      }
      else if ((filesize - self->offset) < (u64)offset) {
        return (RESULT(u64, vfs_err)) ERR(VFS_EINVAL);
      }
      else {
        self->offset += (u64)offset;
      }

      break;

    case VFS_SEEK_END:
      if (offset > 0) {
        return (RESULT(u64, vfs_err)) ERR(VFS_EINVAL);
      }
      else {
        u64 mag = (u64)(-(offset + 1)) + 1;

        if (mag > filesize) {
          return (RESULT(u64, vfs_err)) ERR(VFS_EINVAL);
        }

        self->offset = filesize - mag;
      }

      break;
  }

  return (RESULT(u64, vfs_err)) OK(self->offset);
}


static RESULT(vfs_dirent_ptr, vfs_err) initrd__readdir(vfs_file *self, allocator a) {
  (void)self;
  (void)a;

  // TODO:

  return (RESULT(vfs_dirent_ptr, vfs_err)) ERR(VFS_ENOSYS);
}


// MARK: - api

void initrd_load(initrd *self, const_span buffer) {
  assert(self != NULL);

  self->buffer = buffer;
}


vfs_node *initrd_root(initrd *self) {
  assert(self != NULL);

  allocator a = heap_allocator();

  vfs_node     *node    = allocate(a, sizeof(vfs_node));
  node_fs_data *fs_data = allocate(a, sizeof(node_fs_data));

  fs_data->dev     = self;
  fs_data->abspath = str_clone(a, str_literal("."));
  fs_data->header  = ustar_lookup(self->buffer, fs_data->abspath);
  assert_release(fs_data->header != NULL);

  node->ops     = &initrd_node_ops;
  node->type    = VFS_NODETYPE_DIRECTORY;
  node->release = initrd__release_node;
  node->fs_data = fs_data;

  return vfs_node_incref(node);
}
