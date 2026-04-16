#ifdef DEBUG

#include <slightx/assert.h>

#include <slightx/mem/bytes.h>
#include <slightx/mem/str.h>
#include <slightx/mem/arena.h>

#include <slightx/sys/io.h>
#include <slightx/sys/proc.h>


#define is_aligned(value, alignment) !(value & (alignment - 1))
#define is_signed(type)              (((type)->info & 1) != 0)
#define is_int(type)                 (((type)->kind & 0xF) == 1)
#define int_width(type)              (1UL << ((type)->info >> 1))

// MARK: - structs

typedef struct source_location source_location;
struct source_location {
  const char *filename;
  u32 line;
  u32 column;
};


typedef struct type_descriptor type_descriptor;
struct type_descriptor {
  u16  kind;
  u16  info;
  char name[];
};


typedef struct type_mismatch_info type_mismatch_info;
struct type_mismatch_info {
  source_location  location;
  type_descriptor *type;

  usize alignment;
  u8    type_check_kind;
};

typedef struct type_mismatch_info_v1 type_mismatch_info_v1;
struct type_mismatch_info_v1 {
  source_location  location;
  type_descriptor *type;

  u8 alignment;
  u8 type_check_kind;
};

str type_check_kinds[] = {
  str_literal("load of"),
  str_literal("store to"),
  str_literal("reference binding to"),
  str_literal("member access within"),
  str_literal("member call on"),
  str_literal("constructor call on"),
  str_literal("downcast of"),
  str_literal("downcast of"),
  str_literal("upcast of"),
  str_literal("cast to virtual base of"),
};


typedef struct pointer_overflow_info pointer_overflow_info;
struct pointer_overflow_info {
  source_location location;
};


typedef struct invalid_builtin_info invalid_builtin_info;
struct invalid_builtin_info {
  source_location location;

  u8 kind;
};


typedef struct overflow_info overflow_info;
struct overflow_info {
  source_location  location;
  type_descriptor *type;
};


typedef struct shift_out_of_bounds_info shift_out_of_bounds_info;
struct shift_out_of_bounds_info {
  source_location  location;
  type_descriptor *lhs_type;
  type_descriptor *rhs_type;
};


typedef struct out_of_bounds_info out_of_bounds_info;
struct out_of_bounds_info {
  source_location  location;
  type_descriptor *array_type;
  type_descriptor *index_type;
};


typedef struct unreachable_info unreachable_info;
struct unreachable_info {
  source_location location;
};


typedef struct nonnull_arg_info nonnull_arg_info;
struct nonnull_arg_info {
  source_location location;
  source_location param_location;
  int             param_index;
};


typedef struct nonnull_return_info nonnull_return_info;
struct nonnull_return_info {
  source_location location;
  source_location ret_location;
};


typedef struct invalid_value_info invalid_value_info;
struct invalid_value_info {
  source_location  location;
  type_descriptor *type;
};


typedef struct vla_bound_info vla_bound_info;
struct vla_bound_info {
  source_location  location;
  type_descriptor *type;
};


typedef struct function_type_mismatch_info function_type_mismatch_info;
struct function_type_mismatch_info {
  source_location  location;
  type_descriptor *type;
};


// MARK: - utils


[[gnu::no_sanitize("undefined")]]
static void eprintv(const char *fmt, va_list args) {
  char buf[4096] = {};

  arena tmp;
  arena_init(&tmp, make_span(buf, sizeof(buf)));
  allocator a = arena_allocator(&tmp);

  str formatted = str_vformat(a, fmt, args);
  sys_puts(formatted);
  sys_puts(str_literal("\r\n"));
}


[[gnu::no_sanitize("undefined")]]
static void eprint(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  eprintv(fmt, args);
  va_end(args);
}

[[gnu::no_sanitize("undefined")]]
[[noreturn]] static void report_abort(void) {
  sys_exit(127);
}


[[gnu::no_sanitize("undefined")]]
static void report_begin(const source_location *loc, str description) {
  eprint("======== UNDEFINED BEHAVIOR DETECTED ========");
  eprint("Description: %s", description);
  eprint("Location:");
  eprint("  file: %s",   strview_from_cstr(loc->filename));
  eprint("  line: %u",   (u64)loc->line);
  eprint("  column: %u", (u64)loc->column);
  eprint("---------------------------------------------");
}


[[gnu::no_sanitize("undefined")]]
static void report_end(void) {
  eprint("=============================================");
#ifdef CONFIG_UBSAN_ALWAYS_ABORT
  panic("ubsan");
#endif
}


// MARK: - handlers

[[gnu::no_sanitize("undefined")]]
static void report_type_mismatch_nullptr(const type_mismatch_info *info, usize ptr) {
  (void)ptr;

  report_begin(&info->location, str_literal("Null pointer dereference"));

  eprint(
    "%s null pointer of type %s",
    type_check_kinds[info->type_check_kind],
    strview_from_cstr(info->type->name)
  );

  report_end();
}


[[gnu::no_sanitize("undefined")]]
static void report_type_mismatch_alignment(const type_mismatch_info *info, usize ptr) {
  report_begin(&info->location, str_literal("Unaligned access"));

  eprint(
    "%s unaligned pointer %p of type %s (alignment %u)",
    type_check_kinds[info->type_check_kind],
    (void *)ptr,
    strview_from_cstr(info->type->name),
    (u64)info->alignment
  );

  report_end();
}


[[gnu::no_sanitize("undefined")]]
static void report_type_mismatch_objsize(const type_mismatch_info *info, usize ptr) {
  report_begin(&info->location, str_literal("Insufficient object size"));

  eprint(
    "%s address %p with insufficient space for type %s",
    type_check_kinds[info->type_check_kind],
    (void *)ptr,
    strview_from_cstr(info->type->name)
  );

  report_end();
}


[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_type_mismatch(type_mismatch_info *info, usize ptr) {
  if (ptr == 0) {
    report_type_mismatch_nullptr(info, ptr);
  }
  else if (info->alignment != 0 && !is_aligned(ptr, info->alignment)) {
    report_type_mismatch_alignment(info, ptr);
  }
  else {
    report_type_mismatch_objsize(info, ptr);
  }
}

[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_type_mismatch_abort(type_mismatch_info *info, usize ptr) {
  __ubsan_handle_type_mismatch(info, ptr);
  report_abort();
}


[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_type_mismatch_v1(type_mismatch_info_v1 *info, usize ptr) {
  type_mismatch_info info_v0 = {
    .type            = info->type,
    .alignment       = 1UL << info->alignment,
    .type_check_kind = info->type_check_kind,
  };
  memcpy(&info_v0.location, &info->location, sizeof(source_location));

  __ubsan_handle_type_mismatch(&info_v0, ptr);
}

[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_type_mismatch_v1_abort(type_mismatch_info_v1 *info, usize ptr) {
  __ubsan_handle_type_mismatch_v1(info, ptr);
  report_abort();
}


[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_pointer_overflow(pointer_overflow_info *info, usize base, usize result) {
  report_begin(&info->location, str_literal("Pointer overflow"));

  eprint("Pointer operation overflow %p to %p", (void *)base, (void *)result);

  report_end();
}

[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_pointer_overflow_abort(pointer_overflow_info *info, usize base, usize result) {
  __ubsan_handle_pointer_overflow(info, base, result);
  report_abort();
}


[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_invalid_builtin(invalid_builtin_info *info) {
  report_begin(&info->location, str_literal("Invalid builtin"));

  switch (info->kind) {
    case 0:
      eprint("Passed 0 to clz()");
      break;

    case 1:
      eprint("Passed 0 to ctz()");
      break;

    default:
      eprint("Passed 0 to <unknown> (kind: %u)", (u64)info->kind);
      break;
  }

  report_end();
}

[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_invalid_builtin_abort(invalid_builtin_info *info) {
  __ubsan_handle_invalid_builtin(info);
  report_abort();
}


[[gnu::no_sanitize("undefined")]]
static void report_int_overflow(const overflow_info *info, usize lhs, usize rhs, str op) {
  if (is_signed(info->type)) {
    report_begin(&info->location, str_literal("Signed Integer overflow"));
    eprint(
      "%d %s %d can't be represented in type %s",
      (i64)lhs, op, (i64)rhs,
      strview_from_cstr(info->type->name)
    );
  }
  else {
    report_begin(&info->location, str_literal("Unsigned Integer overflow"));
    eprint(
      "%u %s %u can't be represented in type %s",
      (u64)lhs, op, (u64)rhs,
      strview_from_cstr(info->type->name)
    );
  }

  report_end();
}


[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_add_overflow(overflow_info *info, usize lhs, usize rhs) {
  report_int_overflow(info, lhs, rhs, str_literal("+"));
}

[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_add_overflow_abort(overflow_info *info, usize lhs, usize rhs) {
  __ubsan_handle_add_overflow(info, lhs, rhs);
  report_abort();
}


[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_sub_overflow(overflow_info *info, usize lhs, usize rhs) {
  report_int_overflow(info, lhs, rhs, str_literal("-"));
}

[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_sub_overflow_abort(overflow_info *info, usize lhs, usize rhs) {
  __ubsan_handle_sub_overflow(info, lhs, rhs);
  report_abort();
}


[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_mul_overflow(overflow_info *info, usize lhs, usize rhs) {
  report_int_overflow(info, lhs, rhs, str_literal("*"));
}

[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_mul_overflow_abort(overflow_info *info, usize lhs, usize rhs) {
  __ubsan_handle_mul_overflow(info, lhs, rhs);
  report_abort();
}


[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_negate_overflow(overflow_info *info, usize operand) {
  report_begin(&info->location, str_literal("Negation overflow"));

  if (is_signed(info->type)) {
    eprint(
      "-%d can't be represented in type %s",
      (i64)operand,
      strview_from_cstr(info->type->name)
    );
  }
  else {
    eprint(
      "-%u can't be represented in type %s",
      (u64)operand,
      strview_from_cstr(info->type->name)
    );
  }

  report_end();
}

[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_negate_overflow_abort(overflow_info *info, usize operand) {
  __ubsan_handle_negate_overflow(info, operand);
  report_abort();
}


[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_divrem_overflow(overflow_info *info, usize lhs, usize rhs) {
  report_begin(&info->location, str_literal("Division overflow"));

  if (is_signed(info->type) && (isize)rhs == -1) {
    eprint(
      "%d / -1 can't be represented in type %s",
      (i64)lhs,
      strview_from_cstr(info->type->name)
    );
  }
  else if (rhs == 0) {
    eprint("division by zero");
  }
  else {
    eprint(
      "%u / %u can't be represented in type %s",
      (u64)lhs, (u64)rhs,
      strview_from_cstr(info->type->name)
    );
  }

  report_end();
}

[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_divrem_overflow_abort(overflow_info *info, usize lhs, usize rhs) {
  __ubsan_handle_divrem_overflow(info, lhs, rhs);
  report_abort();
}

[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_shift_out_of_bounds(shift_out_of_bounds_info *info, usize lhs, usize rhs) {
  report_begin(&info->location, str_literal("Shift out of bounds"));

  if (is_signed(info->rhs_type) && (isize)rhs < 0) {
    eprint("shift exponent %d is negative", (i64)rhs);
  }
  else if ((usize)rhs >= int_width(info->lhs_type)) {
    eprint(
      "shift exponent %d is too large for type %s",
      (i64)rhs,
      strview_from_cstr(info->lhs_type->name)
    );
  }
  else if (is_signed(info->lhs_type) && (isize)lhs < 0) {
    eprint(
      "left shift of negative type %s",
      strview_from_cstr(info->lhs_type->name)
    );
  }
  else {
    eprint(
      "left shift of %u by %u places cannot be represented in type %s",
      (u64)lhs, (u64)rhs,
      strview_from_cstr(info->lhs_type->name)
    );
  }

  report_end();
}

[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_shift_out_of_bounds_abort(shift_out_of_bounds_info *info, usize lhs, usize rhs) {
  __ubsan_handle_shift_out_of_bounds(info, lhs, rhs);
  report_abort();
}


[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_out_of_bounds(out_of_bounds_info *info, usize index) {
  report_begin(&info->location, str_literal("Out of bounds"));

  if (is_signed(info->index_type)) {
    eprint(
      "index %d out of bounds for type %s",
      (i64)index,
      strview_from_cstr(info->array_type->name)
    );
  }
  else {
    eprint(
      "index %u out of bounds for type %s",
      (u64)index,
      strview_from_cstr(info->array_type->name)
    );
  }

  report_end();
}

[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_out_of_bounds_abort(out_of_bounds_info *info, usize index) {
  __ubsan_handle_out_of_bounds(info, index);
  report_abort();
}


[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_builtin_unreachable(unreachable_info *info) {
  report_begin(&info->location, str_literal("Unreachable"));
  eprint("Execution reached an unreachable code path");
  report_end();
  report_abort();
}


[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_nonnull_arg(nonnull_arg_info *info) {
  report_begin(&info->location, str_literal("Non-null argument"));

  eprint(
    "parameter %u at %s:%u:%u is declared as non-null but null was passed",
    (u64)info->param_index,
    strview_from_cstr(info->param_location.filename),
    (u64)info->param_location.line,
    (u64)info->param_location.column
  );

  report_end();
}

[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_nonnull_arg_abort(nonnull_arg_info *info) {
  __ubsan_handle_nonnull_arg(info);
  report_abort();
}


[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_nonnull_return(nonnull_return_info *info) {
  report_begin(&info->location, str_literal("Non-null return"));

  eprint(
    "function at %s:%u:%u is declared as non-null returning but returned null",
    strview_from_cstr(info->ret_location.filename),
    (u64)info->ret_location.line,
    (u64)info->ret_location.column
  );

  report_end();
}

[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_nonnull_return_abort(nonnull_return_info *info) {
  __ubsan_handle_nonnull_return(info);
  report_abort();
}

[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_nonnull_return_v1(nonnull_return_info *info) {
  __ubsan_handle_nonnull_return(info);
}

[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_nonnull_return_v1_abort(nonnull_return_info *info) {
  __ubsan_handle_nonnull_return_abort(info);
}


[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_load_invalid_value(invalid_value_info *info, usize value) {
  report_begin(&info->location, str_literal("Invalid value"));

  if (is_int(info->type)) {
    if (is_signed(info->type)) {
      eprint(
        "load of value %d, which is not a valid value for type %s",
        (i64)value,
        strview_from_cstr(info->type->name)
      );
    }
    else {
      eprint(
        "load of value %u, which is not a valid value for type %s",
        (u64)value,
        strview_from_cstr(info->type->name)
      );
    }
  }
  else {
    eprint(
      "load of value, which is not a valid value for type %s",
      strview_from_cstr(info->type->name)
    );
  }

  report_end();
}

[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_load_invalid_value_abort(invalid_value_info *info, usize value) {
  __ubsan_handle_load_invalid_value(info, value);
  report_abort();
}


[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_vla_bound_not_positive(vla_bound_info *info, isize bound) {
  report_begin(&info->location, str_literal("Variable length array bound not positive"));

  eprint("variable length array bound %d is not positive", (i64)bound);

  report_end();
}

[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_vla_bound_not_positive_abort(vla_bound_info *info, isize bound) {
  __ubsan_handle_vla_bound_not_positive(info, bound);
  report_abort();
}


[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_function_type_mismatch(function_type_mismatch_info *info, usize func_ptr) {
  report_begin(&info->location, str_literal("Function type mismatch"));

  eprint(
    "call through function pointer %p to incorrect function type %s",
    (void *)func_ptr,
    strview_from_cstr(info->type->name)
  );

  report_end();
}

[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_function_type_mismatch_abort(function_type_mismatch_info *info, usize func_ptr) {
  __ubsan_handle_function_type_mismatch(info, func_ptr);
  report_abort();
}

#endif
