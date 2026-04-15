#pragma once

#include <slightx/types.h>
#include <slightx/mem/str.h>
#include <slightx/mem/span.h>

#include <vfs/types.h>
#include <vfs/path.h>


typedef enum {
  VFS_MSG_UNKNOWN = 0,

  VFS_MSG_LOOKUP,
  VFS_MSG_CREATE,
  VFS_MSG_LINK,
  VFS_MSG_UNLINK,
  VFS_MSG_RENAME,
  VFS_MSG_SYMLINK,
  VFS_MSG_READLINK,
  VFS_MSG_STAT,
  VFS_MSG_CHMOD,
  VFS_MSG_CHOWN,
  VFS_MSG_OPEN,

  VFS_MSG_CLOSE,
  VFS_MSG_READ,
  VFS_MSG_WRITE,
  VFS_MSG_SEEK,
  VFS_MSG_READDIR,
  VFS_MSG_TRUNCATE,
  VFS_MSG_FLUSH,

  VFS_MSG_MOUNT,
  VFS_MSG_UMOUNT,
} vfs_msg;


// MARK: - node ops

typedef struct vfs_req_lookup vfs_req_lookup;
struct vfs_req_lookup {
  vfs_msg _;

  vfs_path path;
  bool     nofollow;
};

typedef struct vfs_resp_lookup vfs_resp_lookup;
struct vfs_resp_lookup {
  vfs_msg _;

  bool is_ok;
  union {
    vfs_node node;
    vfs_err  err;
  };
};


typedef struct vfs_req_create vfs_req_create;
struct vfs_req_create {
  vfs_msg _;

  vfs_path      path;
  vfs_node_type type;
  vfs_perms     perms;
};

typedef struct vfs_resp_create vfs_resp_create;
struct vfs_resp_create {
  vfs_msg _;

  bool is_ok;
  union {
    vfs_node node;
    vfs_err  err;
  };
};


typedef struct vfs_req_link vfs_req_link;
struct vfs_req_link {
  vfs_msg _;

  vfs_path path;
  vfs_node target;
};

typedef struct vfs_resp_link vfs_resp_link;
struct vfs_resp_link {
  vfs_msg _;

  bool    is_ok;
  vfs_err err;
};


typedef struct vfs_req_unlink vfs_req_unlink;
struct vfs_req_unlink {
  vfs_msg _;

  vfs_path path;
};

typedef struct vfs_resp_unlink vfs_resp_unlink;
struct vfs_resp_unlink {
  vfs_msg _;

  bool    is_ok;
  vfs_err err;
};


typedef struct vfs_req_rename vfs_req_rename;
struct vfs_req_rename {
  vfs_msg _;

  vfs_path oldpath;
  vfs_path newpath;
};

typedef struct vfs_resp_rename vfs_resp_rename;
struct vfs_resp_rename {
  vfs_msg _;

  bool    is_ok;
  vfs_err err;
};


typedef struct vfs_req_symlink vfs_req_symlink;
struct vfs_req_symlink {
  vfs_msg _;

  vfs_path target;
  vfs_path path;
};

typedef struct vfs_resp_symlink vfs_resp_symlink;
struct vfs_resp_symlink {
  vfs_msg _;

  bool    is_ok;
  vfs_err err;
};


typedef struct vfs_req_readlink vfs_req_readlink;
struct vfs_req_readlink {
  vfs_msg _;

  vfs_path path;
};

typedef struct vfs_resp_readlink vfs_resp_readlink;
struct vfs_resp_readlink {
  vfs_msg _;

  bool is_ok;
  union {
    vfs_path target;
    vfs_err  err;
  };
};


typedef struct vfs_req_stat vfs_req_stat;
struct vfs_req_stat {
  vfs_msg _;

  vfs_path path;
};

typedef struct vfs_resp_stat vfs_resp_stat;
struct vfs_resp_stat {
  vfs_msg _;

  bool is_ok;
  union {
    vfs_stat stat;
    vfs_err  err;
  };
};


typedef struct vfs_req_chmod vfs_req_chmod;
struct vfs_req_chmod {
  vfs_msg _;

  vfs_path path;
  vfs_mode mode;
};

typedef struct vfs_resp_chmod vfs_resp_chmod;
struct vfs_resp_chmod {
  vfs_msg _;

  bool    is_ok;
  vfs_err err;
};


typedef struct vfs_req_chown vfs_req_chown;
struct vfs_req_chown {
  vfs_msg _;

  vfs_path path;
  u32      uid;
  u32      gid;
};

typedef struct vfs_resp_chown vfs_resp_chown;
struct vfs_resp_chown {
  vfs_msg _;

  bool    is_ok;
  vfs_err err;
};


typedef struct vfs_req_open vfs_req_open;
struct vfs_req_open {
  vfs_msg _;

  vfs_path path;
  u32      flags;
};

typedef struct vfs_resp_open vfs_resp_open;
struct vfs_resp_open {
  vfs_msg _;

  bool is_ok;
  union {
    vfs_file file;
    vfs_err  err;
  };
};


// MARK: - file ops

typedef struct vfs_req_close vfs_req_close;
struct vfs_req_close {
  vfs_msg _;

  vfs_file file;
};

typedef struct vfs_resp_close vfs_resp_close;
struct vfs_resp_close {
  vfs_msg _;

  bool    is_ok;
  vfs_err err;
};


typedef struct vfs_req_read vfs_req_read;
struct vfs_req_read {
  vfs_msg _;

  vfs_file file;
  usize    nbytes;
};

typedef struct vfs_resp_read vfs_resp_read;
struct vfs_resp_read {
  vfs_msg _;

  bool is_ok;
  union {
    struct {
      usize nbytes;
      u8    data[VFS_BUFFER_MAX];
    };
    vfs_err err;
  };
};


typedef struct vfs_req_write vfs_req_write;
struct vfs_req_write {
  vfs_msg _;

  vfs_file file;
  usize    nbytes;
  u8       data[VFS_BUFFER_MAX];
};

typedef struct vfs_resp_write vfs_resp_write;
struct vfs_resp_write {
  vfs_msg _;

  bool is_ok;
  union {
    usize   nbytes;
    vfs_err err;
  };
};


typedef struct vfs_req_seek vfs_req_seek;
struct vfs_req_seek {
  vfs_msg _;

  vfs_file        file;
  i64             offset;
  vfs_seek_whence whence;
};

typedef struct vfs_resp_seek vfs_resp_seek;
struct vfs_resp_seek {
  vfs_msg _;

  bool is_ok;
  union {
    u64     offset;
    vfs_err err;
  };
};


typedef struct vfs_req_readdir vfs_req_readdir;
struct vfs_req_readdir {
  vfs_msg _;

  vfs_file file;
};

typedef struct vfs_resp_readdir vfs_resp_readdir;
struct vfs_resp_readdir {
  vfs_msg _;

  bool is_ok;
  union {
    vfs_dirent next_entry;
    vfs_err    err;
  };
};


typedef struct vfs_req_truncate vfs_req_truncate;
struct vfs_req_truncate {
  vfs_msg _;

  vfs_file file;
  u64      size;
};

typedef struct vfs_resp_truncate vfs_resp_truncate;
struct vfs_resp_truncate {
  vfs_msg _;

  bool    is_ok;
  vfs_err err;
};


typedef struct vfs_req_flush vfs_req_flush;
struct vfs_req_flush {
  vfs_msg _;

  vfs_file file;
};

typedef struct vfs_resp_flush vfs_resp_flush;
struct vfs_resp_flush {
  vfs_msg _;

  bool    is_ok;
  vfs_err err;
};


// MARK: - mount ops

typedef struct vfs_req_mount vfs_req_mount;
struct vfs_req_mount {
  vfs_msg _;

  vfs_path path;
  vfs_node root;
};

typedef struct vfs_resp_mount vfs_resp_mount;
struct vfs_resp_mount {
  vfs_msg _;

  bool    is_ok;
  vfs_err err;
};


typedef struct vfs_req_umount vfs_req_umount;
struct vfs_req_umount {
  vfs_msg _;

  vfs_path path;
};

typedef struct vfs_resp_umount vfs_resp_umount;
struct vfs_resp_umount {
  vfs_msg _;

  bool    is_ok;
  vfs_err err;
};
