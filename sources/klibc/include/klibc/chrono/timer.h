#pragma once

#include <klibc/types.h>


typedef struct timer timer;
struct timer {
  u64 (*get_time_ns)(void);

  u64  started_at;
  u64  paused_at;

  bool started;
  bool paused;
};


void timer_init(timer *self, u64 (*get_time_ns)(void));

void timer_start (timer *self);
void timer_stop  (timer *self);
void timer_pause (timer *self);
void timer_resume(timer *self);

bool timer_is_started(timer *self);
bool timer_is_paused (timer *self);

u64 timer_elapsed_ns(timer *self);

