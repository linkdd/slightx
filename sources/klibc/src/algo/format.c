#include <klibc/algo/format.h>
#include <klibc/algo/conv.h>
#include <klibc/mem/str.h>
#include <klibc/assert.h>


void formatter_apply(formatter *self, const char *fmt, ...) {
  assert(self != NULL);

  va_list ap;
  va_start(ap, fmt);
  formatter_apply_v(self, fmt, ap);
  va_end(ap);
}


void formatter_apply_v(formatter *self, const char *fmt, va_list args) {
  assert(self != NULL);

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
          self->consume(self->udata, str_literal("%"));
          return;

        case 'd':
          ival = va_arg(args, i64);
          if (ival < 0) {
            uival    = -ival;
            negative = true;
          }
          else {
            uival = ival;
          }

          itoa(&num_buf, uival, 10);

          if (negative) { self->consume(self->udata, str_literal("-")); }
          self->consume(self->udata, num_buf);

          break;

        case 'u':
          uival = va_arg(args, u64);
          itoa(&num_buf, uival, 10);
          self->consume(self->udata, num_buf);

          break;

        case 'b':
          uival = va_arg(args, u64);
          itoa(&num_buf, uival, 2);

          self->consume(self->udata, str_literal("0b"));
          self->consume(self->udata, num_buf);

          break;

        case 'p':
          self->consume(self->udata, str_literal("*"));
        case 'x':
        case 'X':
          uival = va_arg(args, u64);
          itoa(&num_buf, uival, 16);

          self->consume(self->udata, str_literal("0x"));
          self->consume(self->udata, num_buf);

          break;

        case 's':
          self->consume(self->udata, va_arg(args, str));
          break;

        case 'c': {
          char ch = (char)va_arg(args, i32);
          self->consume(self->udata, str_char(ch));
          break;
        }

        case '%':
          self->consume(self->udata, str_literal("%"));
          break;

        default:
          self->consume(self->udata, str_literal("%"));
          self->consume(self->udata, str_char(c));
          break;
      }
    }
    else {
      self->consume(self->udata, str_char(c));
    }
  }
}
