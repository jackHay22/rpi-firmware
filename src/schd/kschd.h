/*
 * (C) Jack Hay, Apr 2021
 */

#ifndef _SCHD_KSCHD_H
#define _SCHD_KSCHD_H

#include <stdint.h>
#include <stddef.h>
#include <stdnoreturn.h>
#include "kpcb.h"

//page size
#define THREAD_SIZE 4096

/**
 * Initialize the kernel process scheduler
 */
void init_kschd();

/**
 * Set the status of the current running process
 * If the status is set to PROC_WAITING a new proc is scheduled
 * @param stat the status
 */
void set_curr_proc_state(kproc_stat stat);

/**
 * Free a process
 * @param pcb the process control block to free
 */
void free_kproc(kpcb_t* pcb);

/**
 * Find a process by process id
 * @param  kpid the id of the process
 * @param  pcb  the pcb ptr set by the call on success
 * @return      0 on success, else > 0
 */
uint8_t get_proc_kpid(uint64_t kpid, kpcb_t** pcb);

/**
 * Create a kernel thread
 * @param kthread_fn the function to execute
 * @param tname      the name of this kernel thread
 * @param argc       number of args passed to thread
 * @param argv       the args passed to the kernel thread
 * @param flags      flags
 * @return the new process id
 */
uint64_t kthread_create(uint64_t kthread_fn,
                        const char *tname,
                        uint8_t argc,
                        char *argv[],
                        uint8_t flags);

/**
 * Start the scheduler
 */
void kschd_start();


#endif /*_SCHD_KSCHD_H*/
