#include <kernel/vfs/error.h>


str vfs_strerror(vfs_err status) {
  switch (status) {
    case VFS_UNKNOWN:   return str_literal("Unknown error");
    case VFS_ENOTFOUND: return str_literal("Not found");
    case VFS_EEXISTS:   return str_literal("Already exists");
    case VFS_EACCESS:   return str_literal("Permission denied");
    case VFS_EIO:       return str_literal("I/O error");
    case VFS_ENOMEM:    return str_literal("Out of memory");
    case VFS_ENOSPC:    return str_literal("No space left on device");
    case VFS_EINVAL:    return str_literal("Invalid argument");
    case VFS_EISDIR:    return str_literal("Is a directory");
    case VFS_ENOTDIR:   return str_literal("Not a directory");
    case VFS_ENOTEMPTY: return str_literal("Directory not empty");
    case VFS_ELOOP:     return str_literal("Too many symbolic links encountered");
    case VFS_EBUSY:     return str_literal("Device or resource busy");
    case VFS_ENOSYS:    return str_literal("Function not implemented");
  }

  unreachable();
}
