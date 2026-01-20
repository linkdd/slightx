#pragma once

#include <klibc/types.h>

#include <klibc/sync/lock.h>


typedef struct wq_handle wq_handle;
struct wq_handle {
  void (*wake)(void *udata);

  void *udata;
};

typedef struct wq_entry wq_entry;
struct wq_entry {
  wq_handle handle;

  struct {
    wq_entry *prev;
    wq_entry *next;
  } siblings;
};

typedef struct waitqueue waitqueue;
struct waitqueue {
  spinlock lock;

  wq_entry *head;
  wq_entry *tail;
};


void waitqueue_entry_init(wq_entry *self, wq_handle handle);

void waitqueue_init(waitqueue *self);

void waitqueue_add(waitqueue *self, wq_entry *entry);
void waitqueue_del(waitqueue *self, wq_entry *entry);

void waitqueue_wake_one(waitqueue *self);
void waitqueue_wake_all(waitqueue *self);

bool waitqueue_is_empty(waitqueue *self);
