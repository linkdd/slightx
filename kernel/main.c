#include <multiboot.h>
#include <asm/asm.h>
#include <screen.h>
#include <util.h>
#include <mem/gdt.h>
#include <mem/idt.h>
#include <mem/paging.h>
#include <timer.h>
#include <task.h>
#include <fs/fs.h>
#include <fs/initrd.h>
#include <sys/syscall.h>

extern uint32_t placement_address;
uint32_t initial_esp;

void ls_root (void)
{
    int i = 0;
    struct dirent *node = 0;

    while ((node = readdir_fs (fs_root, i)) != 0)
    {
        struct fs_node *fsnode;

        printk ("- Found file /%s ", node->name);
        fsnode = finddir_fs (fs_root, node->name);

        if ((fsnode->flags & 0x7) == FS_DIRECTORY)
            printk ("(directory)\n");
        else
            printk ("(file)\n");

        i++;
    }
}


int kmain (struct multiboot_t *mboot, uint magic, uint32_t initial_stack)
{
    uint32_t initrd_location;
    uint32_t initrd_end;
    extern char kattr;

    initial_esp = initial_stack;

    clear ();
    printk ("SlightX: Starting up...\n");

    if (magic != MULTIBOOT_HEADER_MAGIC)
    {
        PANIC ("MultiBoot Header magic number isn't correct.");
    }

    kattr = 0x0F; printk ("# %s\n\n", (char *) mboot->cmdline); kattr = 0x07;

    printk ("Initializing System Structures : ");

    printk ("GDT...");
    init_gdt ();
    printk ("\b\b\b, PIC...");
    init_pic ();
    printk ("\b\b\b, IDT...");
    init_idt ();
    kattr = 0x02; printk ("\b\b\b [ ok ]\n"); kattr = 0x07;

    sti ();
    printk ("Initializing PIT to 100Hz...\n");
    init_timer (50);

    ASSERT (mboot->mods_count > 0);
    initrd_location = *((uint32_t *)mboot->mods_addr);
    initrd_end = *(uint32_t *)(mboot->mods_addr + 4);
    placement_address = initrd_end;

    printk ("Initializing System : ");

    printk ("Paging...");
    init_paging ();
    printk ("\b\b\b, Multitasking...");
    init_tasking ();
    printk ("\b\b\b, initrd...");
    fs_root = init_initrd (initrd_location);
    printk ("\b\b\b, syscalls...");
    init_syscalls ();
    kattr = 0x02; printk ("\b\b\b [ ok ]\n"); kattr = 0x07;

    printk ("Entering runlevel : 3\n");
    switch_to_user_mode ();

    /* syscall_write ("SlightX: System loaded.\n"); 
    syscall_dummy (); */

    return 0;
}
