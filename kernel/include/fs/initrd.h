#ifndef __INITRD_H
#define __INITRD_H

/*!
 * \file initrd.h
 * \brief Defines the interface for and structures relating to the initial ramdisk.
 * \author David Delassus
 *
 * The initial ramdisk, or initrd is a temporary file system
 * commonly used in the boot process of the Linux kernel. It
 * is typically used for making preparations before the real
 * root file system can be mounted.
 */

#include <types.h>
#include <fs/fs.h>

/*! \struct initrd_header_t */
struct initrd_header_t
{
    uint32_t nfiles; /*!< The number of files in the ramdisk. */
};

/*! \struct initrd_file_header_t */
struct initrd_file_header_t
{
    uint8_t magic;      /*!< Magic number, for error checking. */
    int8_t name[64];    /*!< Filename. */
    uint32_t offset;    /*!< Offset in the initrd that the file starts. */
    uint32_t length;    /*!< Length ofthe file. */
};

struct fs_node *init_initrd (uint32_t location);

#endif /* __INITRD_H */
