#include <limine.h>
#include <flanterm.h>
#include <flanterm_backends/fb.h>

#include <klibc/mem/bytes.h>
#include <klibc/sync/lock.h>

#include <kernel/drivers/console.h>
#include <kernel/halt.h>


static spinlock console_lock = {};

static struct flanterm_context *ft_ctx = NULL;


void console_init(void) {
  LIMINE_GET_RESP(framebuffer);

  bool fb_available = (
    framebuffer_response != NULL &&
    framebuffer_response->framebuffer_count > 0
  );

  if (!fb_available) {
    halt();
  }

  spinlock_init(&console_lock);

  struct limine_framebuffer *fb = framebuffer_response->framebuffers[0];

  ft_ctx = flanterm_fb_init(
    /* malloc */ NULL,
    /* free */   NULL,
    fb->address,

    fb->width,
    fb->height,
    fb->pitch,

    fb->red_mask_size,   fb->red_mask_shift,
    fb->green_mask_size, fb->green_mask_shift,
    fb->blue_mask_size,  fb->blue_mask_shift,

    /* canvas */ NULL,

    /* ansi_colours */        NULL,
    /* ansi_bright_colours */ NULL,

    /* default_bg */        NULL,
    /* default_fg */        NULL,
    /* default_bg_bright */ NULL,
    /* default_fg_bright */ NULL,

    /* font */         NULL,
    /* font_width */   0,
    /* font_height */  0,
    /* font_spacing */ 1,
    /* font_scale_x */ 0,
    /* font_scale_y */ 0,

    /* margin */       0,
    FLANTERM_FB_ROTATE_0
  );

  if (ft_ctx == NULL) {
    halt();
  }
}


void console_write(str s) {
  spinlock_acquire(&console_lock);
  if (ft_ctx != NULL) {
    flanterm_write(ft_ctx, s.data, s.length);
  }
  spinlock_release(&console_lock);
}
