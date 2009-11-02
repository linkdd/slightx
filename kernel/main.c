#include <multiboot.h>
#include <screen.h>
#include <mem.h>

void kmain (struct multiboot_info *mbi, unsigned int magic)
{
	*((char *) 0xB8000) = 'a';
	clear ();

	if (magic != MULTIBOOT_HEADER_MAGIC)
	{
		print ("PANIC: MultiBoot Header magic number isn't correct.\n");
		print ("System halted.\n");
		asm ("hlt");
	}

	init_paging ();
	init_gdt ();
	init_idt ();

	while (1);
	return;
}
