#ifndef __MULTIBOOT_H
#define __MULTIBOOT_H

/*!
 * \file multiboot.h
 * \brief Declares the multiboot info structure
 * \author David Delassus
 */

#include <types.h>

#define MULTIBOOT_HEADER_MAGIC 0x2BADB002

#define MULTIBOOT_FLAG_MEM     0x001
#define MULTIBOOT_FLAG_DEVICE  0x002
#define MULTIBOOT_FLAG_CMDLINE 0x004
#define MULTIBOOT_FLAG_MODS    0x008
#define MULTIBOOT_FLAG_AOUT    0x010
#define MULTIBOOT_FLAG_ELF     0x020
#define MULTIBOOT_FLAG_MMAP    0x040
#define MULTIBOOT_FLAG_CONFIG  0x080
#define MULTIBOOT_FLAG_LOADER  0x100
#define MULTIBOOT_FLAG_APM     0x200
#define MULTIBOOT_FLAG_VBE     0x400

/*!
 * \struct multiboot_t
 * \brief Multiboot Header Structure
 *
 * When a Multiboot compliant bootloader load our kernel,
 * it inits a structure and store it in the EBX register.
 * In loader.asm we pass 3 arguments to the main function :
 * this structure, the multiboot magic number and the initial
 * stack.
 */
struct multiboot_t
{
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;

    struct
    {
        uint32_t num;
        uint32_t size;
        uint32_t addr;
        uint32_t shndx;
    } elf_sec;

    uint32_t mmap_length;
    uint32_t mmap_addr;
    uint32_t drives_length;
    uint32_t drives_addr;
    uint32_t config_table;
    uint32_t boot_loader_name;
    uint32_t apm_table;

    struct
    {
        uint32_t control_info;
        uint32_t mode_info;
        uint32_t mode;
        uint32_t interface_seg;
        uint32_t interface_off;
        uint32_t interface_len;
    } vbe;
}  __attribute__((packed));

#endif /* __MULTIBOOT_H */
