#include <klibc/chrono/timer.h>
#include <klibc/assert.h>


void timer_init(timer *self, u64 (*get_time_ns)(void)) {
  assert(self != NULL);
  assert(get_time_ns != NULL);

  self->get_time_ns = get_time_ns;

  self->started_at = 0;
  self->paused_at  = 0;

  self->started = false;
  self->paused  = false;
}


void timer_start(timer *self) {
  assert(self != NULL);

  self->started_at = self->get_time_ns();
  self->paused_at  = 0;

  self->started = true;
  self->paused  = false;
}


void timer_stop(timer *self) {
  assert(self != NULL);

  self->started_at = 0;
  self->paused_at  = 0;

  self->started = false;
  self->paused  = false;
}


void timer_pause(timer *self) {
  assert(self != NULL);
  assert(self->started);
  assert(!self->paused);

  self->paused_at = self->get_time_ns();
  self->paused    = true;
}


void timer_resume(timer *self) {
  assert(self != NULL);
  assert(self->started);
  assert(self->paused);

  u64 pause_duration = self->get_time_ns() - self->paused_at;
  self->started_at  += pause_duration;
  self->paused_at    = 0;

  self->paused = false;
}


bool timer_is_started(timer *self) {
  assert(self != NULL);

  return self->started;
}


bool timer_is_paused(timer *self) {
  assert(self != NULL);

  return self->paused;
}


u64 timer_elapsed_ns(timer *self) {
  assert(self != NULL);
  assert(self->started);

  if (self->paused) {
    return self->paused_at - self->started_at;
  }
  else {
    return self->get_time_ns() - self->started_at;
  }
}
