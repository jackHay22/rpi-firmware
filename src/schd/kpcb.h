/*
 * (C) Jack Hay, Apr 2021
 */

#ifndef _SCHD_KPCB_H
#define _SCHD_KPCB_H

#include "cpu_context.h"

//process state
typedef struct kproc_state_t {
  //cpu architected state
  struct cpu_context_t regs;
  //whether this process can be preempted
  uint8_t preempt_counter;
  //ticks this process can run for
  int tick_count;
} kproc_state_t;

/*
 * The states that a kernel process can be in
 */
typedef enum kproc_stat {
  PROC_RUNNING,
  PROC_WAITING,
  PROC_WAITABLE,
  PROC_ZOMBIED
} kproc_stat;

/*
 * Process control block for a kernel thread
 */
typedef struct kpcb_t {
  //process state (page in memory)
  kproc_state_t* state;
  //parent process id
  uint64_t kppid;
  //kernel processid
  uint64_t kpid;
  //the priority of this process
  uint8_t priority;
  //flags
  uint8_t flags;
  //the kprocess status
  kproc_stat stat;
  //num args to the kthread
  uint8_t argc;
  //args
  char **argv;
  //the process exit code
  uint8_t exit_code;

  //linked list ptrs
  struct kpcb_t* next;
  struct kpcb_t* prev;
} kpcb_t;

#endif /*_SCHD_KPCB_H*/
