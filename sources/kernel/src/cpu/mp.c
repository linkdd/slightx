#include <limine.h>

#include <klibc/assert.h>

#include <kernel/cpu/mp.h>
#include <kernel/mem/heap.h>


typedef struct mp_context mp_context;
struct mp_context {
  struct {
    usize        count;
    percpu_data *items;
  } cpus_data;

  syncpoint current_syncpoint;
};

static mp_context ctx = {};


static void mp_ap_fn_wrapper(struct limine_mp_info *cpu_info);


void mp_init(void) {
  LIMINE_GET_RESP(mp);
  assert_release(mp_response != NULL);

  syncpoint_init(&ctx.current_syncpoint);
}


void mp_load(void) {
  LIMINE_GET_RESP(mp);
  assert_release(mp_response != NULL);

  allocator a = heap_allocator();

  ctx.cpus_data.count = mp_response->cpu_count;
  ctx.cpus_data.items = allocate_v(a, sizeof(percpu_data), ctx.cpus_data.count);

  for (usize i = 0; i < ctx.cpus_data.count; i++) {
    struct limine_mp_info *cpu_info = mp_response->cpus[i];
    percpu_data           *cpu_data = &ctx.cpus_data.items[i];

    cpu_data->processor_id = i;
    cpu_data->lapic_id     = cpu_info->lapic_id;
    cpu_data->is_bootstrap = cpu_info->lapic_id == mp_response->bsp_lapic_id;

    cpu_data->tss.ist1 = (u64)(&cpu_data->isr_stack[ISR_STACK_SIZE]);
  }
}


void mp_ap_jump(cpu_fn fn) {
  LIMINE_GET_RESP(mp);

  for (usize cpu_idx = 0; cpu_idx < ctx.cpus_data.count; cpu_idx++) {
    struct limine_mp_info *cpu_info = mp_response->cpus[cpu_idx];

    ctx.cpus_data.items[cpu_idx].current_fn = fn;

    cpu_info->goto_address   = mp_ap_fn_wrapper;
    cpu_info->extra_argument = (u64)((uptr)&ctx.cpus_data.items[cpu_idx]);
  }
}


usize mp_get_cpu_count(void) {
  return ctx.cpus_data.count;
}


syncpoint *mp_get_syncpoint(void) {
  return &ctx.current_syncpoint;
}


percpu_data *mp_get_percpu_data(void) {
  return (percpu_data*)percpu_get_segment();
}


percpu_data *mp_get_percpu_data_of(usize processor_id) {
  assert(processor_id < ctx.cpus_data.count);

  return &ctx.cpus_data.items[processor_id];
}


static void mp_ap_fn_wrapper(struct limine_mp_info* cpu_info) {
  uptr segment = (uptr)cpu_info->extra_argument;
  percpu_set_segment(segment);

  percpu_data *cpu_data = mp_get_percpu_data();
  cpu_data->current_fn();
}
