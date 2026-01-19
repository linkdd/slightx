#pragma once

#include <klibc/types.h>
#include <klibc/mem/alloc.h>

#include <kernel/mem/vmm.h>


#define HEAP_SIZE         0x20000000  // 512 MiB
#define HEAP_PAGE_COUNT   (HEAP_SIZE / MM_VIRT_PAGE_SIZE)

void heap_init(void);
void heap_load(void);

allocator heap_allocator(void);
