#pragma once

#include <kernel/mem/addr.h>


virtual_address  hhdm_p2v(physical_address pa);
physical_address hhdm_v2p(virtual_address  va);
