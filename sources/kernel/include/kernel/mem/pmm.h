#pragma once

#include <klibc/types.h>

#include <kernel/mem/addr.h>


#define MM_PHYS_PAGE_SIZE     0x1000 // 4kb


void pmm_init(void);
void pmm_load(void);

physical_address pmm_alloc(usize page_count);
void             pmm_free (physical_address pa, usize page_count);
