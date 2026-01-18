#include <klibc/io/log.h>
#include <klibc/mem/str.h>
#include <klibc/algo/conv.h>


extern void console_write(str s);


void klog(const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  vklog(fmt, ap);
  va_end(ap);
}


void vklog(const char *fmt, va_list ap) {
  for (; *fmt != 0; fmt++) {
    char c = *fmt;

    if (c == '%') {
      fmt++;
      c = *fmt;

      // lowest base is binary, 64 bit numbers require 64 digits + 1 trailing '\0'
      char num_bufmem[65] = {};
      str  num_buf        = {
        .data     = num_bufmem,
        .length   = 0,
        .capacity = sizeof(num_bufmem),
        .owned    = false,
      };

      u64 uival     = 0;
      i64 ival      = 0;
      bool negative = false;

      switch (c) {
        case 0:
          console_write(str_literal("%"));
          return;

        case 'd':
          ival = va_arg(ap, i64);
          if (ival < 0) {
            uival    = -ival;
            negative = true;
          }
          else {
            uival = ival;
          }

          itoa(&num_buf, uival, 10);

          if (negative) { console_write(str_literal("-")); }
          console_write(num_buf);

          break;

        case 'u':
          uival = va_arg(ap, u64);
          itoa(&num_buf, uival, 10);
          console_write(num_buf);

          break;

        case 'b':
          uival = va_arg(ap, u64);
          itoa(&num_buf, uival, 2);

          console_write(str_literal("0b"));
          console_write(num_buf);

          break;

        case 'p':
          console_write(str_literal("*"));
        case 'x':
        case 'X':
          uival = va_arg(ap, u64);
          itoa(&num_buf, uival, 16);

          console_write(str_literal("0x"));
          console_write(num_buf);

          break;

        case 's':
          console_write(va_arg(ap, str));
          break;

        case 'c': {
          char ch = (char)va_arg(ap, i32);
          console_write(str_char(ch));
          break;
        }

        case '%':
          console_write(str_literal("%"));
          break;

        default:
          console_write(str_literal("%"));
          console_write(str_char(c));
          break;
      }
    }
    else {
      console_write(str_char(c));
    }
  }
}
