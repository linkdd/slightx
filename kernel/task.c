#include <task.h>
#include <mem/tss.h>
#include <mem/paging.h>
#include <mem/kheap.h>
#include <asm/asm.h>
#include <util.h>

volatile struct task_t *current_task;   /* currently running task */
volatile struct task_t *ready_queue;    /* start of the task linked list */

/* Some externs are needed to access members in paging.c */
extern struct page_directory_t *kernel_directory;
extern struct page_directory_t *current_directory;
extern void alloc_frame (struct page_t *, int, int);
extern uint32_t initial_esp;
extern uint32_t read_eip (void); /* ASM function */

/* The next available PID (Process ID) */
uint32_t next_pid = 0;

void init_tasking (void)
{
    cli (); /* rather important stuff happening, no interrupts please! */

    move_stack ((void *) 0xE0000000, 0x2000); /* Relocate the stack, so we know where it is */

    /* Initialise the first task (kernel stack) */
    current_task = ready_queue = (struct task_t *) kmalloc (sizeof (struct task_t));
    current_task->id = next_pid++;
    current_task->esp = current_task->ebp = 0;
    current_task->eip = 0;
    current_task->page_directory = current_directory;
    current_task->next = 0;
    current_task->kernel_stack = kmalloc_a (KERNEL_STACK_SIZE);

    sti (); /* Reenable interrupts */
}

void move_stack (void *new_stack_start, uint32_t size)
{
    uint32_t i;
    uint32_t pd_addr;
    uint32_t old_stack_pointer, old_base_pointer;
    uint32_t offset;
    uint32_t new_stack_pointer, new_base_pointer;

    /* Allocate some space for the new stack */
    for (i = (uint32_t) new_stack_start;
         i >= ((uint32_t) new_stack_start - size);
         i -= 0x1000)
    {
        /* General-purpose stack is in user-mode */
        alloc_frame (get_page (i, 1, current_directory), 0 /* user-mode */, i /* is writeable */);
    }

    /* Flush the TLB by reading and writing the page directory address again. */
    asm volatile ("mov %%cr3, %0" : "=r" (pd_addr));
    asm volatile ("mov %0, %%cr3" :: "r" (pd_addr));

    /* Old ESP and EBP, read from registers */
    asm volatile ("mov %%esp, %0" : "=r" (old_stack_pointer));
    asm volatile ("mov %%ebp, %0" : "=r" (old_base_pointer));

    /* Offset to add to old stack addresses to get a new stack address. */
    offset = (uint32_t) new_stack_start - initial_esp;

    /* New ESP and EBP */
    new_stack_pointer = old_stack_pointer + offset;
    new_base_pointer  = old_base_pointer  + offset;

    /* Copy the stack */
    memcpy ((void *) new_stack_pointer, (void *) old_stack_pointer, initial_esp - old_stack_pointer);

    /* backtrace through the original stack, copying new values into the new stack */
    for (i = (uint32_t) new_stack_start; i > (uint32_t) new_stack_start - size; i -= 4)
    {
        uint32_t tmp = * (uint32_t *) i;

        /* If the value of tmp is inside the range of the old stack, assume it is a base pointer
         * and remap it. This will unfortunately remap ANY value in this range, wether they are
         * base pointers or not.
         */
        if ((old_stack_pointer < tmp) && (tmp < initial_esp))
        {
            uint32_t *tmp2 = (uint32_t *) i;
            tmp += offset;
            *tmp2 = tmp;
        }
    }

    /* Change stacks */
    asm volatile ("mov %0, %%esp" :: "r" (new_stack_pointer));
    asm volatile ("mov %0, %%ebp" :: "r" (new_base_pointer));
}

void switch_task (void)
{
    uint32_t esp, ebp, eip;

    /* If we haven't initialised tasking yet, just return */
    if (!current_task)
        return;

    /* Read esp, ebp now for saving later on */
    asm volatile ("mov %%esp, %0" : "=r" (esp));
    asm volatile ("mov %%ebp, %0" : "=r" (ebp));

    /* ReadRead the instruction pointer. We do some cunning logic here:
     * One of two things could have happened when this function exits -
     *   (a) We called the function and it returned the EIP as requested.
     *   (b) We have just switched tasks, and because the saved EIP is essentially
     *       the instruction after read_eip(), it will seem as if read_eip has just
     *       returned.
     * In the second case we need to return immediately. To detect it we put a dummy
     * value in EAX further down at the end of this function. As C returns values in EAX,
     * it will look like the return value is this dummy value! (0x12345).
     */
    eip = read_eip ();

    /* Have we just switched tasks ? */
    if (eip == 0x12345)
        return;

    /* No we didn't switch tasks. Let's save some register values and switch */
    current_task->eip = eip;
    current_task->esp = esp;
    current_task->ebp = ebp;

    /* Get the next task to run */
    current_task = current_task->next;
    /* If we fell off the end of likend list start again at the beginning */
    if (!current_task) current_task = ready_queue;

    eip = current_task->eip;
    esp = current_task->esp;
    ebp = current_task->ebp;

    /* Make sure the memory manager knows we've changed page directory */
    current_directory = current_task->page_directory;

    /* Change pour kernel stack over */
    set_kernel_stack (current_task->kernel_stack + KERNEL_STACK_SIZE);
    /* Here we:
     * - Stop interrupts so we don't get interrupted.
     * - Temporarily put the new EIP location in ECX.
     * - Load the stack and base pointers from the new task struct.
     * - Change page directory to the physical address (physical_addr) of the new directory.
     * - Put a dummy value (0x12345) in EAX so that above we can recognise that we've just
     *   switched task.
     * - Restart interrupts. The STI instruction has a delay - it doesn't take effect until after
     *   the next instruction.
     * - Jump to the location in ECX (remember we put the new EIP in there).
     */
    asm volatile ("             \
        cli;                    \
        mov %0, %%ecx;          \
        mov %1, %%esp;          \
        mov %2, %%ebp;          \
        mov %3, %%cr3;          \
        mov $0x12345, %%eax;    \
        sti;                    \
        jmp *%%ecx              "
        :: "r" (eip), "r"(esp), "r"(ebp), "r"(current_directory->physical_addr));
}

int fork (void)
{
    struct page_directory_t *directory;
    struct task_t *parent_task;
    struct task_t *new_task;
    struct task_t *tmp_task;
    uint32_t eip;

    cli (); /* We are modifying kernel structures, and so cannot be interrupted. */

    parent_task = (struct task_t *) current_task;
    directory = clone_directory (current_directory); /* Clone the address space */

    /* Create a new process */
    new_task = (struct task_t *) kmalloc (sizeof (struct task_t));
    new_task->id = next_pid++;
    new_task->esp = new_task->ebp = 0;
    new_task->eip = 0;
    new_task->page_directory = directory;
    current_task->kernel_stack = kmalloc_a (KERNEL_STACK_SIZE);
    new_task->next = 0;

    /* Add it to the end of the ready_queue.
     * Find the end of the ready queue...
     */
    tmp_task = (struct task_t *) ready_queue;
    while (tmp_task->next)
        tmp_task = tmp_task->next;
    tmp_task->next = new_task; /* ...and extand it */

    eip = read_eip (); /* This will be the entry point for the new process. */

    /* We could be the parent or the child here, check. */
    if (current_task == parent_task)
    {
        uint32_t esp, ebp;

        asm volatile ("mov %%esp, %0" : "=r"(esp));
        asm volatile ("mov %%ebp, %0" : "=r"(ebp));

        new_task->esp = esp;
        new_task->ebp = ebp;
        new_task->eip = eip;

        sti (); /* All finished. Reenable interrupts. */
        return new_task->id; /* And by convention, return the PID of the child. */
    }
    else
    {
        return 0; /* We are the child, by convention return 0. */
    }
}

int getpid (void)
{
    return current_task->id;
}

void switch_to_user_mode (void)
{
    /* Set up our kernel stack */
    set_kernel_stack (current_task->kernel_stack + KERNEL_STACK_SIZE);

    /* Set up a stack structure for switch to user mode. */
    asm volatile ("         \
        cli;                \
        mov $0x23, %ax;     \
        mov %ax, %ds;       \
        mov %ax, %es;       \
        mov %ax, %fs;       \
        mov %ax, %gs;       \
        \
        mov %esp, %eax;     \
        pushl $0x23;        \
        pushl %eax;         \
        pushf;              \
        pushl $0x1B;        \
        push $1f;           \
        iret;               \
      1:                    \
        ");
}
