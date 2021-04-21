/*
 * (C) Jack Hay, Apr 2021
 */

#ifndef _SCHD_CPU_CONTEXT_H
#define _SCHD_CPU_CONTEXT_H

/*
 * Registers that are saved, restored
 * during a context switch
 */
struct cpu_context_t {
  unsigned long x19;
  unsigned long x20;
  unsigned long x21;
  unsigned long x22;
  unsigned long x23;
  unsigned long x24;
  unsigned long x25;
  unsigned long x26;
  unsigned long x27;
  unsigned long x28;
  unsigned long fp;
  unsigned long sp;
  unsigned long pc;
};

#endif /*_SCHD_CPU_CONTEXT_H*/
