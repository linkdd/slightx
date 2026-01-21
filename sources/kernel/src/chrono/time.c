#include <kernel/chrono/time.h>
#include <kernel/cpu/mp.h>


u64 uptime_ns(void) {
  percpu_data *cpu = mp_get_percpu_data();
  return cpu->scheduler.uptime_ns;
}
