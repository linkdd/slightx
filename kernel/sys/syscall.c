#include <sys/syscall.h>
#include <isr.h>
#include <screen.h>

static void syscall_handler (struct registers_t *regs);

DEFN_SYSCALL1 (write, 0, const char *);
DEFN_SYSCALL0 (dummy, 1);

void dummy (void)
{
    return;
}

#define N_SYSCALLS  1
static void *syscalls[N_SYSCALLS] =
{
    &printk,
    &dummy
};

void init_syscalls (void)
{
    set_interrupt_handler (0x80, &syscall_handler);
}

void syscall_handler (struct registers_t *regs)
{
    void *location;
    int ret;

    /* Firstly, check if the requested syscall number is valid.
     * The syscall number is found in EAX.
     */
    if (regs->eax >= N_SYSCALLS)
        return;

    location = syscalls[regs->eax]; /* Get the required syscall location */

    /* We don't know how many parameters the function wants, so we just
     * push them all onto the stack in the correct order. The function will
     * use all the parameters it wants, and we can them all back off afterwards.
     */
    asm volatile ("     \
        push %1;        \
        push %2;        \
        push %3;        \
        push %4;        \
        push %5;        \
        call *%6;       \
        pop %%ebx;      \
        pop %%ebx;      \
        pop %%ebx;      \
        pop %%ebx;      \
        pop %%ebx;      \
      " : "=a" (ret) : "r" (regs->edi), "r" (regs->esi), "r" (regs->edx), "r" (regs->ecx), "r" (regs->ebx), "r" (location));
    regs->eax = ret;
}
