/*
 * (C) Jack Hay, Apr 2021
 */

#ifndef _MMU_KHEAP_H
#define _MMU_KHEAP_H

#include <stdint.h>
#include <stddef.h>

/**
 * Initialize the kernel heap
 * @param heap_start the starting offset of the heap
 * @param heap_size  the size of the heap
 */
void init_kheap(uint64_t heap_start,
                uint64_t heap_size);

/**
 * Allocate some block of heap memory
 * @param size the size of the allocation
 */
void* kmalloc(uint64_t size);

/**
 * Free a heap allocation
 * @param addr the address to free
 */
void kfree(void *addr);

/**
 * Show debug info
 */
void debug_kheap();

#endif /*_MMU_KHEAP_H*/
