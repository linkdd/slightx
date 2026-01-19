#include <kernel/cpu/percpu.h>
#include <kernel/cpu/msr.h>


uptr percpu_get_segment(void) {
  return (uptr)rdmsr(IA32_GS_BASE);
}


void percpu_set_segment(uptr segment) {
  wrmsr(IA32_GS_BASE,        (u64)segment);
  wrmsr(IA32_KERNEL_GS_BASE, (u64)segment);
}
