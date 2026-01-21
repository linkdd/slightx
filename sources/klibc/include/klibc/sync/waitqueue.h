#pragma once

#include <klibc/types.h>

#include <klibc/sync/lock.h>
#include <klibc/sync/waiter.h>


typedef struct waitqueue_item waitqueue_item;
struct waitqueue_item {
  waiter waiter;

  struct {
    waitqueue_item *prev;
    waitqueue_item *next;
  } siblings;
};

typedef struct waitqueue waitqueue;
struct waitqueue {
  spinlock lock;

  waitqueue_item *head;
  waitqueue_item *tail;
};


void waitqueue_item_init(waitqueue_item *self, waiter w);
void waitqueue_init(waitqueue *self);

void waitqueue_add(waitqueue *self, waitqueue_item *entry);
void waitqueue_del(waitqueue *self, waitqueue_item *entry);

void waitqueue_wake_one(waitqueue *self);
void waitqueue_wake_all(waitqueue *self);

bool waitqueue_is_empty(waitqueue *self);
