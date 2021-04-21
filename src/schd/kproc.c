/*
 * (C) Jack Hay, Apr 2021
 */

#include "kproc.h"
#include "kschd.h"
#include "../uart/debug.h"

/*
 * Status flags
 */
#define STATUS_WEXITED  0x8000
#define STATUS_EXITCODE 0xFF

/**
 * Check if wait() status indicates child exited
 * @param  status the status returned by wait()
 * @return        whether the process exited
 */
uint8_t WIFEXITED(uint16_t status) {
  return (uint8_t) (status & STATUS_WEXITED);
}

/**
 * Check the exit status of a process
 * @param  status the status returned by wait()
 * @return        the exit code of the process
 */
uint8_t WEXITSTAT(uint16_t status) {
  return (uint8_t) (status & STATUS_EXITCODE);
}

/**
 * Wait on a kernel process by id
 * @param  kpid    the process id
 * @param  status  ths status (returned)
 * @param  options options (i.e. NOHANG)
 * @return         the process id, 0 if no change, -1 on error
 */
int kwaitpid(uint64_t kpid,
             uint16_t* status,
             uint8_t options) {
  *status = 0;

  kpcb_t* pcb;
  if (get_proc_kpid(kpid,&pcb) == 0) {

    //check if child exited, not yet waited on
    if (pcb->stat == PROC_WAITABLE) {
      *status = *status | STATUS_WEXITED;
      *status = *status | (uint16_t) pcb->exit_code;
      //free the process
      free_kproc(pcb);
      return kpid;

    } else if (pcb->stat == PROC_RUNNING) {
      //block on this process
      if (!(options & WNOHANG)) {
        //block this process
        set_curr_proc_state(PROC_WAITING);

        //process woke from signal, try to wait on kpid again
        return kwaitpid(kpid,status,options);
      }
    }

  } else {
    //process with kpid not found
    return -1;
  }

  //process running
  return 0;
}
