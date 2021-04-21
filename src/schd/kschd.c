/*
 * (C) Jack Hay, Apr 2021
 */

#include "kschd.h"
#include "../kstdlib/kstdlib.h"
#include "../mmu/kheap.h"
#include "../mmu/mmu.h"
#include "../uart/debug.h"

#define PRIORITY_HIGH 0
#define PRIORITY_MED  1
#define PRIORITY_LOW  2

#define FLAG_EXITED     0x80
#define FLAG_TERMINATED 0x40

//switch the cpu state to a new process
void cpu_context_switch(kproc_state_t* prev,
                        kproc_state_t* next);

//sets args, calls run_kproc
void call_proc();

//kernel threads by priority
kpcb_t* KTHREADS_PRI0 = NULL; //highest
kpcb_t* KTHREADS_PRI1 = NULL;
kpcb_t* KTHREADS_PRI2 = NULL; //lowest

//last process id assigned
uint64_t LAST_KPID = 0;

//the current running process
kpcb_t* CURRENT_PROC = NULL;

/**
 * Enable preemption on this process
 */
void ENABLE_PREEMPT() {
  if (CURRENT_PROC != NULL) {
    CURRENT_PROC->state->preempt_counter--;
  }
}

void DISABLE_PREEMPT() {
  if (CURRENT_PROC != NULL) {
    CURRENT_PROC->state->preempt_counter++;
  }
}

/**
 * Idle process
 */
void idle_debug() {
  debug_err("reached idle debug");
  while (1) {}
}

/**
 * Dequeue a process that can be run
 * @return the process to run
 */
kpcb_t* dequeue_kproc() {
  //try at each priority level
  for (int p=0; p<3; p++) {
    //check highest pri processes
    kpcb_t** curr_queue;

    if (p == 0) {
      curr_queue = &KTHREADS_PRI0;
    } else if (p == 1) {
      curr_queue = &KTHREADS_PRI1;
    } else {
      curr_queue = &KTHREADS_PRI2;
    }

    while (*curr_queue != NULL) {
      //check for the first running process
      if ((*curr_queue)->stat == PROC_RUNNING) {
        //the process to dequeue
        kpcb_t *proc = *curr_queue;

        //remove this process from the list
        if ((*curr_queue)->prev != NULL) {
          (*curr_queue)->prev->next = (*curr_queue)->next;
        }
        *curr_queue = (*curr_queue)->next;

        return proc;
      }

      //get the next proc in this queue
      curr_queue = &((*curr_queue)->next);
    }
  }

  //unable to find a candidate
  return CURRENT_PROC;
}

/**
 * Add a process to the runnable queue
 * @param pcb   the process control block
 */
void enqueue_kproc(kpcb_t* pcb) {
  kpcb_t** queue;

  //determine the queue to add to based on priority
  if (pcb->priority == PRIORITY_HIGH) {
    queue = &KTHREADS_PRI0;
  } else if (pcb->priority == PRIORITY_MED) {
    queue = &KTHREADS_PRI1;
  } else {
    queue = &KTHREADS_PRI2;
  }

  //added to end of queue
  pcb->next = NULL;

  if (*queue == NULL) {
    *queue = pcb;
  } else {
    kpcb_t* curr = *queue;
    while (curr->next != NULL) {
      curr = curr->next;
    }
    //add to linked list
    pcb->prev = curr;
    curr->next = pcb;
  }
}

/**
 * Schedule a new process
 */
void kschd_schedule() {
  //swap the process currently being run,
  //enqueue the process that is relinquishing cpu
  kpcb_t *curr = CURRENT_PROC;
  CURRENT_PROC = dequeue_kproc();
  enqueue_kproc(curr);

  //TODO dynamic based on priority
  CURRENT_PROC->state->tick_count = 20;

  //context switch, starts executing new process
  cpu_context_switch(curr->state, CURRENT_PROC->state);
}

/**
 * Timer interrupt handler
 */
void timer_preempt() {

  /*
   * TODO
   */
  CURRENT_PROC->state->tick_count--;

  if ((CURRENT_PROC->state->tick_count == 0) &&
      (CURRENT_PROC->state->preempt_counter == 0)) {
    kschd_schedule();
  }
}

/**
 * Run a process and clean up once exited
 * @param kpid the process id
 * @param fn   the function to be called
 */
void run_kproc(int kpid, uint64_t fn) {
  //process handler
  int (*handler)(int,char**) = (int (*)(int,char**)) fn;

  uint8_t exit_code = 0;
  //get the process by id
  kpcb_t* proc;
  if (get_proc_kpid(kpid,&proc) == 0) {
    //Process can be preempted
    ENABLE_PREEMPT();
    //call the handler
    exit_code = handler(proc->argc,proc->argv);
    //should not preempt process during cleanup, reschedule
    DISABLE_PREEMPT();

  } else {
    debug_val("kpid",kpid);
    debug_err("could not find process by id on start");
  }

  //reap the process

  //set the exit code and flags
  CURRENT_PROC->exit_code = exit_code;
  CURRENT_PROC->flags = CURRENT_PROC->flags | FLAG_EXITED;

  //wake the parent
  kpcb_t* pproc;
  if (get_proc_kpid(CURRENT_PROC->kppid,&pproc) == 0) {
    CURRENT_PROC->stat = PROC_WAITABLE;

    if (pproc->stat == PROC_WAITING) {
      //set parent runnable (wakeup from wait())
      pproc->stat = PROC_RUNNING;
    }
  } else {
    //zombied
    CURRENT_PROC->stat = PROC_ZOMBIED;
  }
  //TODO add to parent waitable queue

  //find a new process to schedule
  kschd_schedule();
}

/**
 * Initialize the kernel process scheduler
 */
void init_kschd() {
  //create idle process
  kpcb_t* idle = (kpcb_t*) kmalloc(sizeof(kpcb_t));
  idle->state = (kproc_state_t*) palloc();
  idle->state->regs.x19 = (uint64_t) run_kproc;
  idle->state->regs.x20 = 0;
  idle->state->regs.x21 = (uint64_t) idle_debug;
  idle->state->regs.pc = (uint64_t) call_proc;
  idle->state->regs.sp = (uint64_t) idle->state + THREAD_SIZE;
  idle->state->preempt_counter = 0;
  idle->state->tick_count = 0;
  idle->kppid = -1;
  idle->kpid = 0;
  idle->priority = PRIORITY_LOW;
  idle->flags = 0;
  idle->stat = PROC_RUNNING;
  idle->argc = 0;
  idle->argv = NULL;
  idle->exit_code = 0;
  idle->next = NULL;
  idle->prev = NULL;

  //add to runnable queue
  enqueue_kproc(idle);
}

/**
 * Set the status of the current running process
 * If the status is set to PROC_WAITING a new proc is scheduled
 * @param stat the status
 */
void set_curr_proc_state(kproc_stat stat) {
  DISABLE_PREEMPT();
  CURRENT_PROC->stat = stat;
  if (stat == PROC_WAITING) {
    //schedule a new task
    kschd_schedule();
  }
  ENABLE_PREEMPT();
}

/**
 * Free a process
 * @param pcb the process control block to free
 */
void free_kproc(kpcb_t* pcb) {
  DISABLE_PREEMPT();
  if (pcb->prev != NULL) {
    pcb->prev->next = pcb->next;
  }
  if (pcb->next != NULL) {
    pcb->next->prev = pcb->prev;
  }

  //free the memory allocations
  pfree(pcb->state);
  kfree(pcb);
  ENABLE_PREEMPT();
}

/**
 * Find a process by process id
 * @param  kpid the id of the process
 * @param  pcb  the pcb ptr set by the call on success
 * @return      0 on success, else > 0
 */
uint8_t get_proc_kpid(uint64_t kpid, kpcb_t** pcb) {
  DISABLE_PREEMPT();

  if ((CURRENT_PROC != NULL) && (CURRENT_PROC->kpid == kpid)) {
    *pcb = CURRENT_PROC;
    ENABLE_PREEMPT();
    return 0;
  }

  for (int p=0; p<3; p++) {
    //check highest pri processes
    kpcb_t* curr_queue;

    if (p == 0) {
      curr_queue = KTHREADS_PRI0;
    } else if (p == 1) {
      curr_queue = KTHREADS_PRI1;
    } else {
      curr_queue = KTHREADS_PRI2;
    }

    //look for the process by id
    while (curr_queue != NULL) {
      if (curr_queue->kpid == kpid) {
        *pcb = curr_queue;
        ENABLE_PREEMPT();
        return 0;
      }
    }
  }
  ENABLE_PREEMPT();
  return 1;
}

/**
 * Create a kernel thread
 * @param kthread_fn the function to execute
 * @param tname      the name of this kernel thread
 * @param argc       number of args passed to thread
 * @param argv       the args passed to the kernel thread
 * @param flags      flags
 * @return the new processid
 */
uint64_t kthread_create(uint64_t kthread_fn,
                        const char *tname,
                        uint8_t argc,
                        char *argv[],
                        uint8_t flags) {
  DISABLE_PREEMPT();

  //get the next processid
  LAST_KPID++;

  //allocate a new kernel pcb
  kpcb_t* new_proc = (kpcb_t*) kmalloc(sizeof(kpcb_t));
  new_proc->state = (kproc_state_t*) palloc();

  //parent is the current running process
  new_proc->kppid = CURRENT_PROC->kpid;
  new_proc->kpid = LAST_KPID;
  new_proc->priority = PRIORITY_HIGH;
  new_proc->flags = flags;
  new_proc->stat = PROC_RUNNING;
  new_proc->argc = argc + 1;
  new_proc->argv = (char**) kmalloc(sizeof(char*) * (argc + 1));
  new_proc->exit_code = 0;

  //first arg is name or process
  uint32_t slen = strlen(tname);
  new_proc->argv[0] = (char*) kmalloc(slen);
  memcpy(new_proc->argv[0],tname,slen);

  if (argc > 0) {
    //copy args
    for (uint8_t i=0; i<argc; i++) {
      slen = strlen(argv[i]);
      new_proc->argv[i+1] = (char*) kmalloc(slen);
      memcpy(new_proc->argv[i+1],argv[i],slen);
    }
  }

  //cpu state
  new_proc->state->regs.x19 = (uint64_t) run_kproc;
  new_proc->state->regs.x20 = LAST_KPID;
  new_proc->state->regs.x21 = kthread_fn;
  new_proc->state->regs.pc = (uint64_t) call_proc;
  new_proc->state->regs.sp = (uint64_t) new_proc->state + THREAD_SIZE;
  new_proc->state->preempt_counter = 0;
  new_proc->state->tick_count = 0;

  //add this process to the runnable queue
  enqueue_kproc(new_proc);

  //reenable preemption on this process
  ENABLE_PREEMPT();

  return LAST_KPID;
}

/**
 * Start the scheduler
 * Runs highest priority process
 */
void kschd_start() {
  kpcb_t *startup_proc = dequeue_kproc();
  CURRENT_PROC = startup_proc;
  run_kproc(startup_proc->kpid,
            startup_proc->state->regs.x21);
}
