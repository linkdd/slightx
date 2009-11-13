#include <mem/kheap.h>
#include <mem/paging.h>
#include <util.h>

extern uint32_t end; /* defined in the linker script. */
uint32_t placement_address = (uint32_t) &end;

extern struct page_directory_t *kernel_directory;
struct heap_t *kheap = 0;

uint32_t kmalloc_int (uint32_t sz, int align, uint32_t *phys)
{
    if (kheap != 0)
    {
        void *addr = alloc (sz, (uint8_t) align, kheap);
        if (phys != 0)
        {
            struct page_t *page = get_page ((uint32_t) addr, 0, kernel_directory);
            *phys = page->frame * 0x1000 + ((uint32_t)addr & 0xFFF);
        }
        return (uint32_t) addr;
    }
    else
    {
        uint32_t tmp;

        if (align == 1 && (placement_address & 0xFFFFF000) )
        {
            /* Align the placement address */
            placement_address &= 0xFFFFF000;
            placement_address += 0x1000;
        }
        if (phys)
        {
            *phys = placement_address;
        }

        tmp = placement_address;
        placement_address += sz;
        return tmp;
    }
}

void kfree (void *p)
{
    free(p, kheap);
}

uint32_t kmalloc_a (uint32_t sz)
{
    return kmalloc_int (sz, 1, 0);
}

uint32_t kmalloc_p (uint32_t sz, uint32_t *phys)
{
    return kmalloc_int (sz, 0, phys);
}

uint32_t kmalloc_ap (uint32_t sz, uint32_t *phys)
{
    return kmalloc_int (sz, 1, phys);
}

uint32_t kmalloc (uint32_t sz)
{
    return kmalloc_int (sz, 0, 0);
}

static void expand (uint32_t new_size, struct heap_t *heap)
{
    uint32_t old_size, i;

    ASSERT (new_size > heap->end_address - heap->start_address); /* Sanity check. */

    /* Get the nearest following page boundary. */
    if ((new_size & 0xFFFFF000) != 0)
    {
        new_size &= 0xFFFFF000;
        new_size += 0x1000;
    }

    // Make sure we are not overreaching ourselves.
    ASSERT (heap->start_address+new_size <= heap->max_address);

    // This should always be on a page boundary.
    old_size = heap->end_address-heap->start_address;

    i = old_size;
    while (i < new_size)
    {
        alloc_frame (get_page (heap->start_address + i, 1, kernel_directory),
                    (heap->supervisor) ? 1 : 0,
                    (heap->readonly) ? 0 : 1);
        i += 0x1000; /* page size */
    }
    heap->end_address = heap->start_address+new_size;
}

static uint32_t contract (uint32_t new_size, struct heap_t *heap)
{
    uint32_t old_size, i;

    ASSERT (new_size < heap->end_address-heap->start_address); /* Sanity check. */

    /* Get the nearest following page boundary. */
    if (new_size & 0x1000)
    {
        new_size &= 0x1000;
        new_size += 0x1000;
    }

    /* Don't contract too far! */
    if (new_size < HEAP_MIN_SIZE)
        new_size = HEAP_MIN_SIZE;

    old_size = heap->end_address-heap->start_address;

    i = old_size - 0x1000;
    while (new_size < i)
    {
        free_frame (get_page (heap->start_address + i, 0, kernel_directory));
        i -= 0x1000;
    }

    heap->end_address = heap->start_address + new_size;
    return new_size;
}

static int32_t find_smallest_hole (uint32_t size, uint8_t page_align, struct heap_t *heap)
{
    /* Find the smallest hole that will fit. */
    uint32_t iterator = 0;
    while (iterator < heap->index.size)
    {
        struct header_t *header = (struct header_t *) lookup_ordered_array (iterator, &heap->index);

        /* If the user has requested the memory be page-aligned */
        if (page_align > 0)
        {
            /* Page-align the starting point of this header. */
            uint32_t location = (uint32_t) header;
            int32_t offset = 0;
            int32_t hole_size;

            if (((location + sizeof (struct header_t)) & 0xFFFFF000) != 0)
            {
                offset = 0x1000 - (location + sizeof (struct header_t)) % 0x1000;
            }

            hole_size = (int32_t) header->size - offset;
            /* Can we fit now? */
            if (hole_size >= (int32_t) size)
                break;
        }
        else if (header->size >= size)
            break;

        iterator++;
    }
    /* Why did the loop exit? */
    if (iterator == heap->index.size)
        return -1; /* We got to the end and didn't find anything. */
    else
        return iterator;
}

static int8_t header_t_less_than (void *a, void *b)
{
    return (((struct header_t *) a)->size < ((struct header_t *) b)->size) ? 1 : 0;
}

struct heap_t *create_heap (uint32_t start, uint32_t end_addr, uint32_t max, uint8_t supervisor, uint8_t readonly)
{
    struct heap_t *heap = (struct heap_t *) kmalloc (sizeof (struct heap_t));
    struct header_t *hole;

    /* All our assumptions are made on startAddress and endAddress being page-aligned. */
    ASSERT (start % 0x1000 == 0);
    ASSERT (end_addr % 0x1000 == 0);

    /* Initialise the index. */
    heap->index = place_ordered_array ((void *) start, HEAP_INDEX_SIZE, &header_t_less_than);

    /* Shift the start address forward to resemble where we can start putting data. */
    start += sizeof (type_t) * HEAP_INDEX_SIZE;

    /* Make sure the start address is page-aligned. */
    if ((start & 0xFFFFF000) != 0)
    {
        start &= 0xFFFFF000;
        start += 0x1000;
    }
    /* Write the start, end and max addresses into the heap structure. */
    heap->start_address = start;
    heap->end_address = end_addr;
    heap->max_address = max;
    heap->supervisor = supervisor;
    heap->readonly = readonly;

    // We start off with one large hole in the index.
    hole = (struct header_t *) start;
    hole->size = end_addr-start;
    hole->magic = HEAP_MAGIC;
    hole->is_hole = 1;
    insert_ordered_array ((void *) hole, &heap->index);

    return heap;
}

void *alloc (uint32_t size, uint8_t page_align, struct heap_t *heap)
{
    struct header_t *orig_hole_header;
    uint32_t orig_hole_pos, orig_hole_size;

    struct header_t *block_header;
    struct footer_t *block_footer;

    /* Make sure we take the size of header/footer into account. */
    uint32_t new_size = size + sizeof (struct header_t) + sizeof (struct footer_t);
    /* Find the smallest hole that will fit. */
    int32_t iterator = find_smallest_hole(new_size, page_align, heap);

    if (iterator == -1) /* If we didn't find a suitable hole */
    {
        /* Save some previous data. */
        uint32_t old_length = heap->end_address - heap->start_address;
        uint32_t old_end_address = heap->end_address;
        uint32_t new_length, idx, value;

        /* We need to allocate some more space. */
        expand (old_length + new_size, heap);
        new_length = heap->end_address-heap->start_address;

        /* Find the endmost header. (Not endmost in size, but in location). */
        iterator = 0;
        /* Vars to hold the index of, and value of, the endmost header found so far. */
        idx = -1;
        value = 0x0;
        while (iterator < heap->index.size)
        {
            uint32_t tmp = (uint32_t) lookup_ordered_array (iterator, &heap->index);
            if (tmp > value)
            {
                value = tmp;
                idx = iterator;
            }
            iterator++;
        }

        /* If we didn't find ANY headers, we need to add one. */
        if (idx == -1)
        {
            struct header_t *header = (struct header_t *) old_end_address;
            struct footer_t *footer;

            header->magic = HEAP_MAGIC;
            header->size = new_length - old_length;
            header->is_hole = 1;

            footer = (struct footer_t *) (old_end_address + header->size - sizeof (struct footer_t));
            footer->magic = HEAP_MAGIC;
            footer->header = header;
            insert_ordered_array ((void *)header, &heap->index);
        }
        else
        {
            /* The last header needs adjusting. */
            struct header_t *header = lookup_ordered_array(idx, &heap->index);
            struct footer_t *footer;

            header->size += new_length - old_length;

            /* Rewrite the footer. */
            footer = (struct footer_t *) ((uint32_t) header + header->size - sizeof (struct footer_t));
            footer->header = header;
            footer->magic = HEAP_MAGIC;
        }
        /* We now have enough space. Recurse, and call the function again. */
        return alloc (size, page_align, heap);
    }

    orig_hole_header = (struct header_t *) lookup_ordered_array (iterator, &heap->index);
    orig_hole_pos = (uint32_t) orig_hole_header;
    orig_hole_size = orig_hole_header->size;

    /* Here we work out if we should split the hole we found into two parts.
     * Is the original hole size - requested hole size less than the overhead for adding a new hole?
     */
    if (orig_hole_size-new_size < sizeof (struct header_t) + sizeof (struct footer_t))
    {
        /* Then just increase the requested size to the size of the hole we found. */
        size += orig_hole_size-new_size;
        new_size = orig_hole_size;
    }

    /* If we need to page-align the data, do it now and make a new hole in front of our block. */
    if (page_align && orig_hole_pos&0xFFFFF000)
    {
        uint32_t new_location = orig_hole_pos + 0x1000 - (orig_hole_pos & 0xFFF) - sizeof (struct header_t);
        struct header_t *hole_header = (struct header_t *) orig_hole_pos;
        struct footer_t *hole_footer;

        hole_header->size     = 0x1000 - (orig_hole_pos & 0xFFF) - sizeof(struct header_t);
        hole_header->magic    = HEAP_MAGIC;
        hole_header->is_hole  = 1;

        hole_footer         = (struct footer_t *)((uint32_t) new_location - sizeof (struct footer_t) );
        hole_footer->magic  = HEAP_MAGIC;
        hole_footer->header = hole_header;
        orig_hole_pos       = new_location;
        orig_hole_size      = orig_hole_size - hole_header->size;
    }
    else
    {
        /* Else we don't need this hole any more, delete it from the index. */
        remove_ordered_array (iterator, &heap->index);
    }

    /* Overwrite the original header... */
    block_header          = (struct header_t *)orig_hole_pos;
    block_header->magic   = HEAP_MAGIC;
    block_header->is_hole = 0;
    block_header->size    = new_size;

    /* ...And the footer */
    block_footer         = (struct footer_t *) (orig_hole_pos + sizeof (struct header_t) + size);
    block_footer->magic  = HEAP_MAGIC;
    block_footer->header = block_header;

    /* We may need to write a new hole after the allocated block.
     * We do this only if the new hole would have positive size...
     */
    if (orig_hole_size - new_size > 0)
    {
        struct header_t *hole_header = (struct header_t *)(orig_hole_pos + sizeof (struct header_t) + size + sizeof (struct footer_t));
        struct footer_t *hole_footer;

        hole_header->magic   = HEAP_MAGIC;
        hole_header->is_hole = 1;
        hole_header->size    = orig_hole_size - new_size;

        hole_footer = (struct footer_t *)((uint32_t) hole_header + orig_hole_size - new_size - sizeof (struct footer_t) );

        if ((uint32_t) hole_footer < heap->end_address)
        {
            hole_footer->magic = HEAP_MAGIC;
            hole_footer->header = hole_header;
        }
        /* Put the new hole in the index */
        insert_ordered_array ((void *) hole_header, &heap->index);
    }

    /* ...And we're done! */
    return (void *)((uint32_t) block_header + sizeof (struct header_t));
}

void free (void *p, struct heap_t *heap)
{
    struct header_t *header, *test_header;
    struct footer_t *footer, *test_footer;
    char do_add;

    /* Exit gracefully for null pointers. */
    if (p == 0)
        return;

    /* Get the header and footer associated with this pointer. */
    header = (struct header_t *)((uint32_t) p - sizeof (struct header_t));
    footer = (struct footer_t *)((uint32_t) header + header->size - sizeof (struct footer_t));

    /* Sanity checks. */
    ASSERT (header->magic == HEAP_MAGIC);
    ASSERT (footer->magic == HEAP_MAGIC);

    header->is_hole = 1; /* Make us a hole. */

    /* Do we want to add this header into the 'free holes' index? */
    do_add = 1;

    /* Unify left
     * If the thing immediately to the left of us is a footer...
     */
    test_footer = (struct footer_t *) ((uint32_t) header - sizeof (struct footer_t));

    if (test_footer->magic == HEAP_MAGIC &&
        test_footer->header->is_hole == 1)
    {
        uint32_t cache_size = header->size; /* Cache our current size. */
        header = test_footer->header;       /* Rewrite our header with the new one. */
        footer->header = header;            /* Rewrite our footer to point to the new header. */
        header->size += cache_size;         /* Change the size. */
        do_add = 0;                         /* Since this header is already in the index, we don't want to add it again. */
    }

    /* Unify right
     * If the thing immediately to the right of us is a header...
     */
    test_header = (struct header_t *)((uint32_t) footer + sizeof (struct footer_t));

    if (test_header->magic == HEAP_MAGIC &&
        test_header->is_hole)
    {
        uint32_t iterator = 0;

        header->size += test_header->size; /* Increase our size. */
        test_footer = (struct footer_t *)((uint32_t) test_header + /* Rewrite it's footer to point to our header. */
                                          test_header->size - sizeof (struct footer_t));
        footer = test_footer;

        /* Find and remove this header from the index. */
        while ((iterator < heap->index.size) &&
               (lookup_ordered_array (iterator, &heap->index) != (void *) test_header))
        {
            iterator++;
        }

        ASSERT (iterator < heap->index.size); /* Make sure we actually found the item. */
        remove_ordered_array(iterator, &heap->index); /* Remove it. */
    }

    /* If the footer location is the end address, we can contract. */
    if ((uint32_t) footer + sizeof (struct footer_t) == heap->end_address)
    {
        uint32_t old_length = heap->end_address-heap->start_address;
        uint32_t new_length = contract ((uint32_t) header - heap->start_address, heap);

        /* Check how big we will be after resizing. */
        if (header->size - (old_length-new_length) > 0)
        {
            /* We will still exist, so resize us. */
            header->size -= old_length-new_length;
            footer = (struct footer_t *)((uint32_t) header + header->size - sizeof (struct footer_t));
            footer->magic = HEAP_MAGIC;
            footer->header = header;
        }
        else
        {
            /* We will no longer exist :(. Remove us from the index. */
            uint32_t iterator = 0;

            while ((iterator < heap->index.size) &&
                   (lookup_ordered_array(iterator, &heap->index) != (void *) test_header))
            {
                iterator++;
            }

            /* If we didn't find ourselves, we have nothing to remove. */
            if (iterator < heap->index.size)
                remove_ordered_array (iterator, &heap->index);
        }
    }

    /* If required, add us to the index. */
    if (do_add == 1)
        insert_ordered_array ((void *)header, &heap->index);
}
