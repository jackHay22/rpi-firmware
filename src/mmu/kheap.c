/*
 * (C) Jack Hay, Apr 2021
 */

#include "kheap.h"
#include "../uart/debug.h"
#include "mmu.h"

//allocation flags
#define FLAG_ALLOCATED 0x80

//allocations must be 8 bytes minimum
#define MIN_ALLOC_SIZE 8

uint64_t TOTAL_HEAP_ALLOC = 0;
uint64_t TOTAL_KHEAP_CAP = 0;

/*
 * Linked list of heap allocations
 */
typedef struct kheap_alloc_t {
  //heap flags
  uint8_t flags;
  //the size of the allocation
  uint64_t size;
  //next and prev ptrs
  struct kheap_alloc_t* next;
  struct kheap_alloc_t* prev;
} kheap_alloc_t;

//allocations
kheap_alloc_t* KHEAP_ALLOCS = NULL;

void show_alloc_table() {
  debug_log("ALLOC TABLE");
  kheap_alloc_t *curr = KHEAP_ALLOCS;
  while (curr != NULL) {
    debug_val("Flags",curr->flags);
    debug_val(" size",curr->size);

    if (curr->flags & FLAG_ALLOCATED) {
      debug_log(" (allocated)");
    } else {
      debug_log(" (free)");
    }

    curr = curr->next;
  }
}

/**
 * Initialize the kernel heap
 * @param heap_start the starting offset of the heap
 * @param heap_size  the size of the heap
 */
void init_kheap(uint64_t heap_start,
                uint64_t heap_size) {
  //zero
  memset((void*) heap_start,0,sizeof(kheap_alloc_t));
  KHEAP_ALLOCS = (kheap_alloc_t*) heap_start;
  //starting heap initialization
  KHEAP_ALLOCS->flags = 0;
  KHEAP_ALLOCS->size = heap_size;
  KHEAP_ALLOCS->next = NULL;
  KHEAP_ALLOCS->prev = NULL;

  TOTAL_KHEAP_CAP = heap_size;
}

/**
 * Allocate some block of heap memory
 * @param size the size of the allocation
 */
void* kmalloc(uint64_t size) {

  if (size <= 0) {
    return NULL;
  }

  TOTAL_HEAP_ALLOC += size;

  //set the minimum allocation size
  if (size < MIN_ALLOC_SIZE) {
    size = MIN_ALLOC_SIZE;
  }

  kheap_alloc_t *curr = KHEAP_ALLOCS;
  while ((curr != NULL) &&
         ((curr->flags & FLAG_ALLOCATED) ||
          (curr->size < (size + sizeof(kheap_alloc_t))))) {
    curr = curr->next;
  }

  if (curr == NULL) {
    //show_alloc_table();
    debug_log("kheap out of space");
    debug_kheap();
    set_errno(ERRNO_KMALLOC);
    return NULL;
  }

  //determine remaining size with metadata storage
  uint64_t avail = curr->size - sizeof(kheap_alloc_t);

  //check if this allocation should be split
  if ((avail - MIN_ALLOC_SIZE) > size) {
    uint64_t rem_alloc_addr = (uint64_t) curr + sizeof(kheap_alloc_t) + size;
    memset((void*)rem_alloc_addr,0,sizeof(kheap_alloc_t));

    //the header representing the remaining space after allocation
    kheap_alloc_t* new_header = (kheap_alloc_t*) rem_alloc_addr;
    new_header->flags = 0;
    new_header->size = curr->size - (sizeof(kheap_alloc_t) + size);

    //update linked list
    new_header->prev = curr;
    new_header->next = curr->next;
    curr->next = new_header;

    if (curr->next != NULL) {
      curr->next->prev = new_header;
    }

    //reduce size of current
    curr->size = size;
  }

  //mark as allocated
  curr->flags = curr->flags | FLAG_ALLOCATED;

  //memory location (after metadata)
  return curr + 1;
}

/**
 * Free a heap allocation
 * @param addr the address to free
 */
void kfree(void *addr) {
  if (addr == NULL) {
    return;
  }

  //locate the segment header
  kheap_alloc_t* header = (kheap_alloc_t*)(uint64_t)addr - sizeof(kheap_alloc_t);

  if ((header != NULL) && (header->flags & FLAG_ALLOCATED)) {
    //mark allocation as free
    header->flags = header->flags & ~FLAG_ALLOCATED;

    //reclaimed space
    TOTAL_HEAP_ALLOC -= header->size;

    //attempt to merge left
    if ((header->prev != NULL) && !(header->prev->flags & FLAG_ALLOCATED)) {
      //expand by freeing space and addr allocation header
      header->prev->size += header->size + sizeof(kheap_alloc_t);
      //update linked list
      header->prev->next = header->next;
      if (header->next != NULL) {
        header->next->prev = header->prev;
      }

      //complete merge
      header = header->prev;
    }

    //attempt to merge right
    if ((header->next != NULL) && !(header->next->flags & FLAG_ALLOCATED)) {
      header->size += header->next->size + sizeof(kheap_alloc_t);

      //update linked list
      if (header->next->next != NULL) {
        header->next->next->prev = header;
      }

      header->next = header->next->next;
    }
  }
}

/**
 * Show debug info
 */
void debug_kheap() {
  debug_val("kheap_used",TOTAL_HEAP_ALLOC);
  debug_val("kheap_cap",TOTAL_KHEAP_CAP);
}
