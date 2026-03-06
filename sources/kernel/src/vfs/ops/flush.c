#include <klibc/assert.h>

#include <kernel/vfs/file-ops.h>


RESULT(UNIT, vfs_err) vfs_flush(vfs_file *file) {
  assert(file != NULL);

  if (file->ops == NULL || file->ops->flush == NULL) {
    return (RESULT(UNIT, vfs_err)) ERR(VFS_ENOSYS);
  }

  return file->ops->flush(file);
}
