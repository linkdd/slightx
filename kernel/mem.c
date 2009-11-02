#include <mem.h>

unsigned long kernelpagedir[1024] __attribute__ ((aligned (4096)));
unsigned long lowpagetable[1024] __attribute__ ((aligned (4096)));

/*
 * \fn void init_paging (void)
 * 
 * This function fill the page directory and the page table,
 * then enables paging by putting the address of the page directory
 * into the CR3 register and setting the 31st bit into the CR0 one
 */
void init_paging (void)
{
	void *p_kernelpagedir = NULL;
	void *p_lowpagetable = NULL;
	int k = 0;

	/* translate the page directory, and the page table,
	 * from virtual addresses to physical addresses
	 */
	p_kernelpagedir = (char *) kernelpagedir + 0x40000000;
	p_lowpagetable = (char *) lowpagetable + 0x40000000;

	for (k = 0; k < 1024; ++k)
	{
		lowpagetable[k] = (k * 4096) | 0x3; /* map the first 4MB of memory into the page table */
		kernelpagedir[k] = 0;               /* and clear the page directory entries */
	}

	/* Fills the addresses 0...4MB and 3072MB...3076MB of the page directory
	 * with the same page table
	 */

	kernelpagedir[0]   = (unsigned long) p_lowpagetable | 0x3;
	kernelpagedir[768] = (unsigned long) p_lowpagetable | 0x3;

	/* Copies the address of the page directory into the CR3 register and enables paging */
	asm volatile (  "mov %0, %%eax\n"
	                "mov %%eax, %%cr3\n"
					"mov %%cr0, %%eax\n"
					"orl $0x80000000, %%eax\n"
					"mov %%eax, %%cr0\n"
					:: "m" (p_kernelpagedir));
	return;
}
