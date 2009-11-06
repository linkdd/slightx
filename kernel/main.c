#include <multiboot.h>
#include <screen.h>
#include <mem.h>

void kmain (struct multiboot_info *mbi, unsigned int magic)
{
	clear ();
	print ("FreeX: Starting up...\n");

	if (magic != MULTIBOOT_HEADER_MAGIC)
	{
		print ("PANIC: MultiBoot Header magic number isn't correct.\n");
		print ("System halted.\n");
		asm ("hlt");
	}

	print ("Initializing System Structures : ");

	print ("Paging ");
	init_paging ();
	print ("GDT ");
	init_gdt ();
	print ("IDT\n");
	init_idt ();

	while (1);
	return;
}
