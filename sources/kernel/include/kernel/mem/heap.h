#pragma once

#include <klibc/types.h>
#include <klibc/mem/alloc.h>

#include <kernel/mem/vmm.h>


void heap_init(void);
void heap_load(void);

allocator heap_allocator(void);
