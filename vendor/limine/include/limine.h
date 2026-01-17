#pragma once

#define LIMINE_API_REVISION 3
#include "../protocol/include/limine.h"


#define LIMINE_REQ(struct_name)             \
  static volatile struct limine_##struct_name##_request struct_name##_request

#define LIMINE_DECL_RESP(struct_name)       \
  struct limine_##struct_name##_response *limine_get_##struct_name##_response(void)

#define LIMINE_DEF_RESP(struct_name)        \
  LIMINE_DECL_RESP(struct_name) {           \
    return struct_name##_request.response;  \
  }

#define LIMINE_GET_RESP(struct_name)                                    \
  struct limine_##struct_name##_response* struct_name##_response = (    \
    limine_get_##struct_name##_response()                               \
  )


LIMINE_DECL_RESP(hhdm);
LIMINE_DECL_RESP(stack_size);
LIMINE_DECL_RESP(bootloader_info);
LIMINE_DECL_RESP(executable_file);
LIMINE_DECL_RESP(module);
LIMINE_DECL_RESP(framebuffer);
LIMINE_DECL_RESP(memmap);
LIMINE_DECL_RESP(paging_mode);
LIMINE_DECL_RESP(mp);
LIMINE_DECL_RESP(rsdp);
LIMINE_DECL_RESP(executable_address);
