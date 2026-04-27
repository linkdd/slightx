#pragma once

#include <klibc/types.h>

#include <kernel/proc/capabilities.h>


static constexpr cap_id CONSOLE_CAP_ID = 0x00010000;

enum : i64 {
  EINVAL = 1,
  ENOMEM = 2,
  ENOSUP = 3,
  EFAULT = 4,
};
