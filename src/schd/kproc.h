/*
 * (C) Jack Hay, Apr 2021
 */

#ifndef _SCHD_KPROC_H
#define _SCHD_KPROC_H

#include <stdint.h>
#include <stddef.h>
#include "kpcb.h"

//optional flags for kwaitpid()
#define WNOHANG 0x80

/**
 * Check if wait() status indicates child exited
 * @param  status the status returned by wait()
 * @return        whether the process exited
 */
uint8_t WIFEXITED(uint16_t status);

/**
 * Check the exit status of a process
 * @param  status the status returned by wait()
 * @return        the exit code of the process
 */
uint8_t WEXITSTAT(uint16_t status);

/**
 * Wait on a kernel process by id
 * @param  kpid    the process id
 * @param  status  ths status (returned)
 * @param  options options (i.e. NOHANG)
 * @return         the process id, 0 if no change, -1 on error
 */
int kwaitpid(uint64_t kpid,
             uint16_t* status,
             uint8_t options);

#endif /*_SCHD_KPROC_H*/
