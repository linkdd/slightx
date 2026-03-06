#pragma once


#define VFS_NAME_MAX          255
#define VFS_PATH_MAX          4096
#define VFS_SYMLINK_DEPTH_MAX 16


typedef struct vfs_node   vfs_node;
typedef struct vfs_file   vfs_file;
typedef struct vfs_dirent vfs_dirent;

typedef struct vfs_stat_desc  vfs_stat_desc;
typedef struct vfs_mount_desc vfs_mount_desc;

typedef vfs_node*   vfs_node_ref;
typedef vfs_file*   vfs_file_ref;
typedef vfs_dirent* vfs_dirent_ptr;

typedef struct vfs_node_ops vfs_node_ops;
typedef struct vfs_file_ops vfs_file_ops;
