#pragma once

#include <slightx/types.h>
#include <slightx/mem/span.h>

#include <slightx/sys/abi.h>


i64 sys_send  (cap_id cap, const_span msg);
i64 sys_call  (cap_id cap, const_span req, span resp);
i64 sys_capctl(cap_id cap, u64 cmd, uptr arg);
