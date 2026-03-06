#pragma once

#include <klibc/sync/lock.h>

#include <kernel/vfs/mount.h>


typedef struct vfs_mtable_entry vfs_mtable_entry;
struct vfs_mtable_entry {
  vfs_mount_desc mount;

  vfs_mtable_entry *prev;
  vfs_mtable_entry *next;
};

typedef struct vfs_mtable vfs_mtable;
struct vfs_mtable {
  spinlock lock;

  vfs_mtable_entry *head;
};


void vfs_mtable_init  (void);
void vfs_mtable_lock  (void);
void vfs_mtable_unlock(void);

vfs_mtable_entry *vfs_mtable_get_head(void);
void              vfs_mtable_set_head(vfs_mtable_entry *entry);
