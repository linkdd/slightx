#include <klibc/assert.h>

#include <kernel/vfs/file-ops.h>


RESULT(usize, vfs_err) vfs_read(vfs_file *file, span buf) {
  assert(file != NULL);
  assert(buf.data != NULL && buf.size > 0);

  if (file->node->type == VFS_NODETYPE_DIRECTORY) {
    return (RESULT(usize, vfs_err)) ERR(VFS_EISDIR);
  }

  if (file->ops == NULL || file->ops->read == NULL) {
    return (RESULT(usize, vfs_err)) ERR(VFS_ENOSYS);
  }

  return file->ops->read(file, buf);
}
