#include <klibc/assert.h>

#include <kernel/vfs/file-ops.h>


RESULT(vfs_dirent_ptr, vfs_err) vfs_readdir(vfs_file *file, allocator a) {
  assert(file != NULL);

  if (file->node->type != VFS_NODETYPE_DIRECTORY) {
    return (RESULT(vfs_dirent_ptr, vfs_err)) ERR(VFS_ENOTDIR);
  }

  if (file->ops == NULL || file->ops->readdir == NULL) {
    return (RESULT(vfs_dirent_ptr, vfs_err)) ERR(VFS_ENOSYS);
  }

  return file->ops->readdir(file, a);
}
