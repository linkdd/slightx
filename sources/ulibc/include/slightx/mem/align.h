#pragma once

#include <slightx/types.h>


bool is_alignment_valid(usize alignment);

bool  is_size_aligned(usize size, usize alignment);
usize align_size_up  (usize size, usize alignment);
usize align_size_down(usize size, usize alignment);

bool is_ptr_aligned(uptr ptr, usize alignment);
uptr align_ptr_up  (uptr ptr, usize alignment);
uptr align_ptr_down(uptr ptr, usize alignment);
