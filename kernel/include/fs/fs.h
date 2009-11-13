#ifndef __FS_H
#define __FS_H

/*!
 * \file fs.h
 * \brief Defines the interface for and structures realting to the VFS (Virtual File System).
 * \author David Delassus
 *
 * Virtual file systems are used to separate the high-level
 * interface to the file system from the low level interfaces
 * that different implementations (FAT, ext3, etc) may require,
 * thus providing transparent access to storage devices from
 * applications. This allows for greater flexibility, specially
 * if one wants to support several file systems.
 */

#include <types.h>

#define FS_FILE        0x01
#define FS_DIRECTORY   0x02
#define FS_CHARDEVICE  0x03
#define FS_BLOCKDEVICE 0x04
#define FS_PIPE        0x05
#define FS_SYMLINK     0x06
#define FS_MOUNTPOINT  0x08 /* Is the file an active mountpoint ? */

struct fs_node;

/* These typedefs define the type of callbacks - called when read/write/open/close
 * are called.
 */
typedef uint32_t (*read_type_t)(struct fs_node *, uint32_t, uint32_t, uint8_t *);
typedef uint32_t (*write_type_t)(struct fs_node *, uint32_t, uint32_t, uint8_t *);
typedef void (*open_type_t)(struct fs_node *);
typedef void (*close_type_t)(struct fs_node *);
typedef struct dirent *(*readdir_type_t)(struct fs_node *, uint32_t);
typedef struct fs_node *(*finddir_type_t)(struct fs_node *, char *name);

/*!
 * \struct fs_node
 * \brief Define a node of the VFS.
 */
struct fs_node
{
    char name[128];         /*!< The filename. */
    uint32_t mask;          /*!< The permissions mask. */
    uint32_t uid;           /*!< The owning user. */
    uint32_t gid;           /*!< The owning group. */
    uint32_t flags;         /*!< Includes the node type. See #defines above. */
    uint32_t inode;         /*!< This is device-specific - provides a way for a filesystem to indentify files. */
    uint32_t length;        /*!< Size of the file, in bytes. */
    uint32_t impl;          /*!< An implementation-defined number. */

    read_type_t read;
    write_type_t write;
    open_type_t open;
    close_type_t close;
    readdir_type_t readdir;
    finddir_type_t finddir;

    struct fs_node *ptr;    /*!< Used by mountpoints and symlinks. */
};

/*!
 * \struct dirent
 * \brief Define a directory in the VFS.
 */
struct dirent
{
    char name[128];     /*!< Filename */
    uint32_t ino;       /*!< Inode number. Required by POSIX. */
};

extern struct fs_node *fs_root; /* The root of the filesystem */

/* Standard read/write/open/close functions. Note that these are all suffixed with
 * _fs to distinguish them from the read/write/open/close which deal with file descriptors
 * , not file nodes.
 */
uint32_t read_fs (struct fs_node *node, uint32_t offset, uint32_t size, uint8_t *buffer);
uint32_t write_fs (struct fs_node *node, uint32_t offset, uint32_t size, uint8_t *buffer);
void open_fs (struct fs_node *node, uint8_t read, uint8_t write);
void close_fs (struct fs_node *node);
struct dirent *readdir_fs (struct fs_node *node, uint32_t index);
struct fs_node *finddir_fs (struct fs_node *node, char *name);

#endif /* __FS_H */
