/*
 * (C) Jack Hay, Apr 2021
 */

#include "uart/debug.h"
#include "uart/uart.h"
#include "mmu/mmu.h"
#include "mmu/kheap.h"
#include "schd/kschd.h"
#include "schd/kproc.h"
#include "display/console.h"
#include "shell/shell.h"
#include <stdnoreturn.h>
#include <stdint.h>
#include <stddef.h>

/**
 * The init process scheduled on startup to complete the
 * rest of the init process
 * @return status
 */
int schd_init_proc(int argc,char *argv[]) {
  debug_log("init display and console");

  if (init_console() == 0) {
    //start another thread to run main kernel mode shell
    uint64_t new_pid = kthread_create((uint64_t)&shell_main,
                                      "kshell", 0, NULL, 0);

    uint16_t stat;
    int child_pid = kwaitpid(new_pid,&stat,0);
    if (child_pid == -1) {
      debug_err("failed to wait");
    }
    debug_val("exit code",WEXITSTAT(stat));
  }

  while (1) {}
  return -1;
}

/**
 * Entry point
 * No return
 */
noreturn void init() {
  //initialize UART
  init_uart();
  debug_log("init");

  //initialize memory mgmt
  debug_log("init mmu");

  //the start of the kernel heap
  uint64_t kheap_start = init_mmu(1024 * 1024 * 1024);
  if (kheap_start <= 0) {
    debug_err("init_mmu failed");
  } else {
    //initialize the kernel heap
    debug_log("init kheap");
    init_kheap(kheap_start,K_HEAP_SIZE_B);

    //initialize the kernel process scheduler
    debug_log("init kschd");
    init_kschd();

    //schedule the initial process
    kthread_create((uint64_t)&schd_init_proc,
                   "init", 0, NULL, 0);

    kschd_start();
    debug_err("kschd_start fell through");
  }
  while (1) {}
}
