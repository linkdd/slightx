#ifndef __TASK_H
#define __TASK_H

/*!
 * \file task.h
 * \brief defines the structures and prototypes needed to multitask.
 * \author David Delassus
 */

#include <mem/paging.h>

#define KERNEL_STACK_SIZE   2048 /* Use a 2Kb kernel stack */

/*!
 * \struct task_t
 * \brief This structures defines a 'task' - a process
 */
struct task_t
{
    int id;                                     /*!< Process ID */
    uint32_t esp, ebp;                          /*!< Stack and base pointers */
    uint32_t eip;                               /*!< Instruction pointer */
    struct page_directory_t *page_directory;    /*!< Page directory */
    uint32_t kernel_stack;                      /*!< Kernel stack location */
    struct task_t *next;                        /*!< The next task in a linked list */
};

void init_tasking (void);
void switch_task (void);
int fork (void);
void move_stack (void *new_stack_start, uint32_t size);
int getpid (void);
void switch_to_user_mode (void);

#endif /* __TASK_H */
