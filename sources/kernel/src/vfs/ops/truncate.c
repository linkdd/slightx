#include <klibc/assert.h>

#include <kernel/vfs/file-ops.h>


RESULT(UNIT, vfs_err) vfs_truncate(vfs_file *file, u64 size) {
  assert(file != NULL);

  if (file->node->type == VFS_NODETYPE_DIRECTORY) {
    return (RESULT(UNIT, vfs_err)) ERR(VFS_EISDIR);
  }

  if (file->ops == NULL || file->ops->truncate == NULL) {
    return (RESULT(UNIT, vfs_err)) ERR(VFS_ENOSYS);
  }

  return file->ops->truncate(file, size);
}
