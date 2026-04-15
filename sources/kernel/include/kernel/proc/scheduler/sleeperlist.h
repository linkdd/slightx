#pragma once

#include <klibc/types.h>

#include <klibc/sync/lock.h>
#include <klibc/sync/waitqueue.h>
#include <klibc/chrono/timer.h>


typedef struct sleeper sleeper;
struct sleeper {
  timer timer;
  u64   duration_ns;

  waitqueue_item *w;

  struct {
    struct sleeper *prev;
    struct sleeper *next;
  } siblings;
};

typedef struct sleeperlist sleeperlist;
struct sleeperlist {
  spinlock  lock;
  sleeper  *head;
  sleeper  *tail;
  usize     count;
};


void sleeperlist_init(sleeperlist *self);
void sleeperlist_add (sleeperlist *self, u64 duration, waitqueue_item *w);
void sleeperlist_tick(sleeperlist *self);
