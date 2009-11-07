#include <multiboot.h>
#include <screen.h>
#include <mem.h>

void kmain (struct multiboot_info *mbi, unsigned int magic)
{
	clear ();
	printk ("FreeX: Starting up...\n");

	if (magic != MULTIBOOT_HEADER_MAGIC)
	{
		printk ("PANIC: MultiBoot Header magic number isn't correct.\n");
		printk ("System halted.\n");
		asm ("hlt");
	}

	printk ("Initializing System Structures : ");

	printk ("Paging, ");
	init_paging ();
	printk ("GDT, ");
	init_gdt ();
	printk ("PIC, ");
	init_pic ();
	printk ("IDT\n\n");
	init_idt ();

	detect_cpu ();	

	while (1);
	return;
}
