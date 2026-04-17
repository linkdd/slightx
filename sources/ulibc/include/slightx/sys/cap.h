#pragma once

#include <slightx/types.h>
#include <slightx/mem/span.h>

#include <slightx/sys/abi.h>


i64 sys_capread  (cap_id cap,       span msg);
i64 sys_capwrite (cap_id cap, const_span msg);
i64 sys_capinvoke(cap_id cap, const_span req, span resp);

i64 sys_capmap   (cap_id cap, void *addr, usize size, u64 flags, void **mapped_addr_ptr);
i64 sys_capctl   (cap_id cap, u64 cmd, uptr arg);

i64 sys_caprelease(cap_id cap);
