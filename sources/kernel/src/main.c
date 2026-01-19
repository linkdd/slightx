#include <limine.h>

#include <klibc/assert.h>
#include <klibc/types.h>
#include <klibc/io/log.h>

#include <kernel/halt.h>

#include <kernel/drivers/console.h>


void kmain(void) {
  console_init();

  LIMINE_GET_RESP(bootloader_info);
  assert_release(bootloader_info_response != NULL);
  klog(
    "bootloader: %s %s\n",
    strview_from_cstr(bootloader_info_response->name),
    strview_from_cstr(bootloader_info_response->version)
  );

  LIMINE_GET_RESP(executable_file);
  assert_release(executable_file_response != NULL);
  klog(
    "[slightx]# %s %s\n",
    strview_from_cstr(executable_file_response->executable_file->path),
    strview_from_cstr(executable_file_response->executable_file->string)
  );

  halt();
}
