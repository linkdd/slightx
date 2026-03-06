#include <klibc/assert.h>

#include <kernel/vfs/file-ops.h>


RESULT(UNIT, vfs_err) vfs_close(vfs_file *file) {
  assert(file != NULL);

  if (file->ops == NULL || file->ops->close == NULL) {
    return (RESULT(UNIT, vfs_err)) ERR(VFS_ENOSYS);
  }

  return file->ops->close(file);
}
