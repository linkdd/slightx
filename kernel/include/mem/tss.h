#ifndef __TSS_H
#define __TSS_H

/*!
 * \file tss.h
 * \brief Task State Segment
 * \author David Delassus
 *
 * The Task State Segment (TSS) is a special data structure
 * for x86 processors which holds information about a task.
 * The TSS is primarily suited for hardware multitasking,
 * where each individual process has its own TSS. In Software
 * multitasking, one or two TSS's are also generally used, as
 * they allow for entering ring0 code after an interrupt.
 */

#include <types.h>

/*!
 * \struct tss_entry_t
 * \brief Structure describing a Task State Segment.
 */
struct tss_entry_t
{
    uint32_t prev_tss;  /*!< The previous TSS - if we used hardware task switching this would form a linked list. */
    uint32_t esp0;      /*!< The stack pointer to load when we change to kernel mode. */
    uint32_t ss0;       /*!< The stack segment to load when we change to kernel mode. */
    uint32_t esp1, ss1; /*!< Unused... */
    uint32_t esp2, ss2;
    uint32_t cr3;
    uint32_t eip, eflags;
    uint32_t eax, ecx, edx, ebx;
    uint32_t esp, ebp, esi, edi;
    uint32_t es;        /*!< The value to load into ES when we change to kernel mode. */
    uint32_t cs;        /*!< The value to load into CS when we change to kernel mode. */
    uint32_t ss;        /*!< The value to load into SS when we change to kernel mode. */
    uint32_t ds;        /*!< The value to load into DS when we change to kernel mode. */
    uint32_t fs;        /*!< The value to load into FS when we change to kernel mode. */
    uint32_t gs;        /*!< The value to load into GS when we change to kernel mode. */
    uint32_t ldt;       /*!< Unused... */
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__ ((packed));

extern void tss_flush (void);
void set_kernel_stack (uint32_t stack);

#endif /* __TSS_H */
