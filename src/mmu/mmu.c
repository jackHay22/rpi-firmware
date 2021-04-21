/*
 * (C) Jack Hay, Apr 2021
 */

#include "mmu.h"
#include "../uart/debug.h"

//end of the kernel image
extern uint8_t __end;

#define FLAG_ALLOCATED 0x80
#define FLAG_KERNEL 0x40
#define FLAG_KHEAP 0x20

/**
 * Defines the allocation of a physical page
 */
typedef struct phy_page_t {
  //flags for this page
  //flags[7] - whether the page is allocated
  //flags[6] - whether the page is used by the kernel
  //flags[5] - whether the page is used by kheap
  uint8_t flags;
  //the address of the page
  uint64_t addr;
} phy_page_t;

//physical pages including the physical page table itself
//in addition to the kernel heap
phy_page_t* P_PAGES_ALL = NULL;
//physical pages not allocated for ptable or kernel heap
uint64_t P_PAGES_OFFSET = 0;

/**
 * Set the memory of some location
 * @param dest  the location
 * @param c     the memory to set
 * @param bytes the number of bytes to set
 */
void memset(void *dest, uint8_t c, uint64_t bytes) {
  uint64_t *d = (uint64_t *)dest;
  while (bytes--) {
    *d++ = c;
  }
}

/**
 * initialize the memory management sytem
 * @param phy_size the size of physical memory
 * @return the starting address of the kernel heap
 */
uint64_t init_mmu(uint64_t phy_size) {
  //the number of pages that can be allocated
  uint64_t p_pages = (phy_size / PAGE_SIZE_B) - 1;

  //the size of the page table in memory
  uint64_t p_pages_s = p_pages * sizeof(phy_page_t);

  //Page table linkedlist resides in memory after the kernel stack
  P_PAGES_ALL = (phy_page_t *)((uint64_t)&__end + K_STACK_SIZE_B);
  memset(P_PAGES_ALL, 0, p_pages_s);

  //calculate where the physical page table ends in memory
  uint64_t p_pages_arr_end = (uint64_t) P_PAGES_ALL + p_pages_s;
  //round up to page size
  if (p_pages_arr_end % PAGE_SIZE_B) {
    //add the difference
    p_pages_arr_end += PAGE_SIZE_B - (p_pages_arr_end % PAGE_SIZE_B);
  }

  //the number of pages that are allocated for the page table itself
  uint64_t ptable_pages = p_pages_arr_end / PAGE_SIZE_B;

  //index of current page
  uint64_t pidx = 0;

  //allocate pages needed to store metadata
  for (pidx=0; pidx<ptable_pages; pidx++) {
    P_PAGES_ALL[pidx].flags = FLAG_ALLOCATED | FLAG_KERNEL;
    P_PAGES_ALL[pidx].addr = p_pages_arr_end + (pidx * PAGE_SIZE_B);
  }

  //allocate pages for the kernel heap
  uint64_t kheap_pages = K_HEAP_SIZE_B / PAGE_SIZE_B;

  //allocate the kernel heap
  for (; pidx<(ptable_pages + kheap_pages); pidx++) {
    P_PAGES_ALL[pidx].flags = FLAG_ALLOCATED | FLAG_KHEAP | FLAG_KERNEL;
    P_PAGES_ALL[pidx].addr = p_pages_arr_end + (pidx * PAGE_SIZE_B);
  }

  //set the start of the non kernel pages
  P_PAGES_OFFSET = pidx;

  //setup metadata for remaining physical pages
  for (; pidx<p_pages; pidx++) {
    P_PAGES_ALL[pidx].flags = 0;
    P_PAGES_ALL[pidx].addr = p_pages_arr_end + (pidx * PAGE_SIZE_B);
  }

  //starting offset of the kernel heap
  return p_pages_arr_end;
}

/**
 * Allocate a page
 */
void* palloc() {
  phy_page_t* curr = &P_PAGES_ALL[P_PAGES_OFFSET];

  //look for a free page
  while ((curr != NULL) && (curr->flags & FLAG_ALLOCATED)) {
    curr++;
  }

  if (curr == NULL) {
    set_errno(ERRNO_PALLOC);
    return NULL;
  }

  if (curr->addr == 0) {
    debug_err("page virtual address was 0");
    return NULL;
  }

  //set page allocated
  curr->flags = curr->flags | FLAG_ALLOCATED;

  //get address of memory, clear
  void *memory = (void*) curr->addr;
  memset(memory, 0, PAGE_SIZE_B);

  //return the allocated memory
  return memory;
}

/**
 * Free an allocated page
 * @param addr the address of the page
 */
void pfree(void *addr) {
  if (addr == NULL) {
    return;
  }

  //locate page in ptable
  phy_page_t *page = P_PAGES_ALL + ((uint64_t)addr / PAGE_SIZE_B);
  //mark free
  page->flags = page->flags & ~FLAG_ALLOCATED;
}
