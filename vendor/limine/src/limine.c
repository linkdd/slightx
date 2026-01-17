#include <limine.h>


LIMINE_REQ(hhdm) = {
  .id       = LIMINE_HHDM_REQUEST_ID,
  .revision = 0
};

LIMINE_REQ(stack_size) = {
  .id         = LIMINE_STACK_SIZE_REQUEST_ID,
  .revision   = 0,
  .stack_size = 16 * 1024 * 1024
};

LIMINE_REQ(bootloader_info) = {
  .id       = LIMINE_BOOTLOADER_INFO_REQUEST_ID,
  .revision = 0
};

LIMINE_REQ(executable_file) = {
  .id       = LIMINE_EXECUTABLE_FILE_REQUEST_ID,
  .revision = 0
};

LIMINE_REQ(module) = {
  .id       = LIMINE_MODULE_REQUEST_ID,
  .revision = 0
};

LIMINE_REQ(framebuffer) = {
  .id       = LIMINE_FRAMEBUFFER_REQUEST_ID,
  .revision = 0,
};

LIMINE_REQ(memmap) = {
  .id       = LIMINE_MEMMAP_REQUEST_ID,
  .revision = 0
};

LIMINE_REQ(paging_mode) = {
  .id       = LIMINE_PAGING_MODE_REQUEST_ID,
  .revision = 0,
  .mode     = LIMINE_PAGING_MODE_X86_64_DEFAULT,
};

LIMINE_REQ(mp) = {
  .id       = LIMINE_MP_REQUEST_ID,
  .revision = 0,
  .flags    = 1,
};

LIMINE_REQ(rsdp) = {
  .id       = LIMINE_RSDP_REQUEST_ID,
  .revision = 0,
};

LIMINE_REQ(executable_address) = {
  .id       = LIMINE_EXECUTABLE_ADDRESS_REQUEST_ID,
  .revision = 0,
};


LIMINE_DEF_RESP(hhdm);
LIMINE_DEF_RESP(stack_size);
LIMINE_DEF_RESP(bootloader_info);
LIMINE_DEF_RESP(executable_file);
LIMINE_DEF_RESP(module);
LIMINE_DEF_RESP(framebuffer);
LIMINE_DEF_RESP(memmap);
LIMINE_DEF_RESP(paging_mode);
LIMINE_DEF_RESP(mp);
LIMINE_DEF_RESP(rsdp);
LIMINE_DEF_RESP(executable_address);
