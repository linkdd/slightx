#include <kernel/args.h>


RESULT(args, UNIT) args_parse(str cmdline) {
  args args = {};

  const struct {
    str  option;
    str *dest;
  } options[] = {
    {str_literal("rdinit"), &args.rdinit},
  };
  constexpr usize option_count = sizeof(options) / sizeof(options[0]);

  usize i = 0;
  while (i < cmdline.length) {
    usize key_start = i;
    usize eq        = key_start;

    while (eq < cmdline.length && cmdline.data[eq] != '=' && cmdline.data[eq] != ' ') {
      eq++;
    }

    if (eq == cmdline.length || cmdline.data[eq] != '=') {
      while (eq < cmdline.length && cmdline.data[eq] != ' ') {
        eq++;
      }

      i = eq + 1;
      continue;
    }

    str   key         = str_slice(cmdline, key_start, eq - key_start);
    usize value_start = eq + 1;
    usize value_end   = value_start;

    while (value_end < cmdline.length && cmdline.data[value_end] != ' ') {
      value_end++;
    }

    str value = str_slice(cmdline, value_start, value_end - value_start);

    for (usize o = 0; o < option_count; ++o) {
      if (str_equal(key, options[o].option)) {
        *options[o].dest = value;
        break;
      }
    }

    i = value_end + 1;
  }

  return (RESULT(args, UNIT)) OK(args);
}
