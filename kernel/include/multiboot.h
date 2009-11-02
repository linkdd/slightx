#ifndef __MULTIBOOT_H
#define __MULTIBOOT_H

#define MULTIBOOT_HEADER_MAGIC  0x2BADB002
#define MULTIBOOT_HEADER_FLAGS  0x00000003

struct multiboot_info
{
	unsigned long flags;
	unsigned long low_mem;
	unsigned long high_mem;
	unsigned long boot_device;
	unsigned long cmdline;
	unsigned long mods_count;
	unsigned long mods_addr;

	struct
	{
		unsigned long num;
		unsigned long size;
		unsigned long addr;
		unsigned long shndx;
	} elf_sec;
};

#endif /* __MULTIBOOT_H */
