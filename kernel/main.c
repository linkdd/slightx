#include <multiboot.h>
#include <screen.h>

void kmain (struct multiboot_info *mbi, unsigned int magic)
{
	clear ();

	if (magic != MULTIBOOT_HEADER_MAGIC)
	{
		print ("PANIC: MultiBoot header magic isn't correct.\n");
		asm ("hlt");
	}

	print ((char *)mbi->cmdline);
	while (1);
	return;
}
