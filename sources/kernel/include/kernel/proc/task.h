#pragma once

#include <klibc/types.h>

#include <klibc/sync/waitqueue.h>

#include <kernel/mem/vmm.h>


typedef u32         tid;
typedef struct task task;


typedef struct task_entrypoint task_entrypoint;
struct task_entrypoint {
  void (*fn)(void *arg);
  void *arg;
};

typedef struct task_pin task_pin;
struct task_pin {
  bool  enabled;
  usize processor_id;
};

typedef enum : u8 {
  TH_TASK_FLAG_DETACHED  = 1 << 0,
  TH_TASK_FLAG_GUARDPAGE = 1 << 1,
  TH_TASK_FLAG_KERNEL    = 1 << 2,
} task_flags;

typedef struct task_desc task_desc;
struct task_desc {
  tid              task_id;
  task            *parent_task;
  usize            kstack_size;
  usize            ustack_size;
  page_map        *parent_pmap;
  task_entrypoint  entrypoint;
  task_pin         pin;
  task_flags       flags;
};

typedef enum : u8 {
  TH_TASK_STATE_NEW = 0,
  TH_TASK_STATE_READY,
  TH_TASK_STATE_RUNNING,
  TH_TASK_STATE_BLOCKED,
  TH_TASK_STATE_ZOMBIE,
} task_state_type;

typedef union task_state_data task_state_data;
union task_state_data {
  struct {
  } new;

  struct {
  } ready;

  struct {
    usize processor_id;
  } running;

  struct {
  } blocked;

  struct {
    i32 exit_code;
  } zombie;
};

typedef struct task_state task_state;
struct task_state {
  task_state_type type;
  task_state_data data;
};

typedef struct task_context task_context;
struct task_context {
  u64 rsp;
  u64 rbp;
  u64 rbx;
  u64 r12;
  u64 r13;
  u64 r14;
  u64 r15;
  u64 rip;
  u64 rflags;
};

typedef struct task_stack task_stack;
struct task_stack {
  void  *base;
  void  *limit;
  usize  size;
};

struct task {
  tid        id;
  task_pin   pin;
  task_flags flags;
  task_state state;

  task_context context;
  task_stack   kstack;
  task_stack   ustack;

  page_map *pmap;

  struct {
    usize last_processor_id;

    struct {
      task *prev;
      task *next;
    } siblings;
  } scheduling;

  struct {
    waitqueue_item  blocker;
    waitqueue       joiners;
    task           *parent;
  } lifecycle;
};


void task_init  (task *self, const task_desc *desc);
void task_deinit(task *self);

void task_set_ready  (task *self);
void task_set_running(task *self, usize processor_id);
void task_set_blocked(task *self);
void task_set_zombie (task *self, i32 exit_code);

extern void task_context_switch(task_context *prev, task_context *next);
