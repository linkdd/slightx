#ifndef __GDT_H
#define __GDT_H

/*!
 * \file gdt.h
 * \brief Global Descriptor Table
 * \author David Delassus
 *
 * The Global Descriptor Table (GDT) is specific to the
 * IA32 architecture. It contains entries telling the CPU
 * about memory segments. A similar Interrupts Descriptor
 * Table exists containing tasks and interrupts descriptors.
 */

#include <types.h>

/*!
 * \struct gdt_entry_t
 * \brief Structure describing a segment's descriptor.
 */
struct gdt_entry_t
{
    uint16_t limit_low;     /*!< The lower 16 bits of the limit. */
    uint16_t base_low;      /*!< The lower 16 bits of the base. */
    uint8_t  base_middle;   /*!< The next 8 bits of the base. */
    uint8_t  access;        /*!< Access flags, determine what ring this segment can be used in. */
    uint8_t  granularity;
    uint8_t  base_high;     /*!< The last 8 bits of the base. */
} __attribute__ ((packed));

/*!
 * \struct gdt_t
 * \brief Address of the GDT (base/limit).
 */
struct gdt_t
{
    uint16_t limit; /*!< The upper 16 bits of all selector limits. */
    uint32_t base;  /*!< The address of the first gdt_entry_t struct. */
} __attribute__ ((packed));

extern void gdt_flush (uint32_t addr);
void init_gdt (void);

#endif /* __GDT_H */
