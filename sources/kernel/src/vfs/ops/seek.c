#include <klibc/assert.h>

#include <kernel/vfs/file-ops.h>


RESULT(u64, vfs_err) vfs_seek(vfs_file *file, i64 offset, vfs_seek_whence whence) {
  assert(file != NULL);

  if (file->node->type == VFS_NODETYPE_DIRECTORY) {
    return (RESULT(u64, vfs_err)) ERR(VFS_EISDIR);
  }

  if (file->ops == NULL || file->ops->seek == NULL) {
    return (RESULT(u64, vfs_err)) ERR(VFS_ENOSYS);
  }

  return file->ops->seek(file, offset, whence);
}
