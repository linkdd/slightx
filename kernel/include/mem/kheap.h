#ifndef __KHEAP_H
#define __KHEAP_H

/*!
 * \file kheap,h
 * \brief Kernel heap functions
 * \author David Delassus
 *
 * Interface for kernel heap functions, also provides
 * a placement malloc() for use before the heap is
 * initialised.
 */

#include <types.h>
#include <array.h>

#define KHEAP_START         0xC0000000
#define KHEAP_INITIAL_SIZE  0x100000

#define HEAP_INDEX_SIZE   0x20000
#define HEAP_MAGIC        0x123890AB
#define HEAP_MIN_SIZE     0x70000

/*! \struct header_t */
struct header_t
{
    uint32_t magic;     /*!< Magic number, used for error checking and identification. */
    uint8_t is_hole;    /*!< 1 if this is a hole. 0 if this is a block. */
    uint32_t size;      /*!< Size of the block, including the end footer. */
};

/*! \struct footer_t */
struct footer_t
{
    uint32_t magic;             /*!< Magic number, same as in header_t */
    struct header_t *header;    /*!< Pointer to the block header */
};

/*! \struct heap_t */
struct heap_t
{
    struct ordered_array_t index;
    uint32_t start_address; /*!< The start of our allocated space. */
    uint32_t end_address;   /*!< The end of our allocated space. May be expanded up to max_address. */
    uint32_t max_address;   /*!< The maximum address the heap can be expanded to */
    uint8_t supervisor;     /*!< Should extra pages requested by us mapped as supervisor-only ? */
    uint8_t readonly;       /*!< Should extra pages requested by us mapped as read-only ? */
};

struct heap_t *create_heap (uint32_t start, uint32_t end_addr, uint32_t max, uint8_t supervisor, uint8_t readonly);
void *alloc (uint32_t size, uint8_t page_align, struct heap_t *heap);
void free (void *p, struct heap_t *heap);

uint32_t kmalloc_int (uint32_t size, int align, uint32_t *phys);
uint32_t kmalloc_a (uint32_t size);
uint32_t kmalloc_p (uint32_t size, uint32_t *phys);
uint32_t kmalloc_ap (uint32_t size, uint32_t *phys);
uint32_t kmalloc (uint32_t size);

void kfree (void *p);

#endif /* __KHEAP_H */
