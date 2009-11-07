#include <asm/asm.h>
#include <multiboot.h>
#include <screen.h>
#include <mem.h>
#include <timer.h>

void kmain (struct multiboot_info *mbi, unsigned int magic)
{
	clear ();
	printk ("SlightX: Starting up...\n");

	if (magic != MULTIBOOT_HEADER_MAGIC)
	{
		printk ("PANIC: MultiBoot Header magic number isn't correct.\n");
		halt ();
	}

	printk ("Initializing System Structures : ");

	printk ("Paging, ");
	init_paging ();
	printk ("GDT, ");
	init_gdt ();
	printk ("IDT, ");
	init_idt ();
	printk ("PIC\n");
	init_pic ();

	printk ("Enable interrupts...\n");
	sti ();

	printk ("Initializing PIT to 50 Hz...\n");
	init_timer (50);


	while (1);
	return;
}
