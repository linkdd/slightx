#include <klibc/assert.h>

#include <kernel/vfs/file-ops.h>


RESULT(u64, vfs_err) vfs_ioctl(vfs_file *file, u64 op, ...) {
  assert(file != NULL);

  if (file->ops == NULL || file->ops->ioctl == NULL) {
    return (RESULT(u64, vfs_err)) ERR(VFS_ENOSYS);
  }

  va_list args;
  va_start(args, op);
  auto res = file->ops->ioctl(file, op, args);
  va_end(args);
  return res;
}
