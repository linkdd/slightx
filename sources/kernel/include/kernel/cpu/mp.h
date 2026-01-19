#pragma once

#include <klibc/types.h>
#include <klibc/sync/lock.h>

#include <kernel/cpu/percpu.h>


void mp_init(void);
void mp_load(void);

void mp_ap_jump(cpu_fn fn);

usize        mp_get_cpu_count     (void);
syncpoint   *mp_get_syncpoint     (void);
percpu_data *mp_get_percpu_data   (void);
percpu_data *mp_get_percpu_data_of(usize processor_id);
