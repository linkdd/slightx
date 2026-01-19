#include <limine.h>

#include <kernel/mem/hhdm.h>


virtual_address hhdm_p2v(physical_address pa) {
  LIMINE_GET_RESP(hhdm);

  if (hhdm_response == NULL) {
    return (virtual_address){ .ptr = NULL };
  }

  return (virtual_address){ .addr = pa.addr + hhdm_response->offset };
}


physical_address hhdm_v2p(virtual_address va) {
  LIMINE_GET_RESP(hhdm);

  if (hhdm_response == NULL) {
    return (physical_address){ .ptr = NULL };
  }

  return (physical_address){ .addr = va.addr - hhdm_response->offset };
}
