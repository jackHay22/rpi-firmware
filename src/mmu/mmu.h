/*
 * (C) Jack Hay, Apr 2021
 */

#ifndef _MMU_MMU_H
#define _MMU_MMU_H

#include <stdint.h>
#include <stddef.h>

/*
 * Page size
 * 4KB
 */
#define PAGE_SIZE_B 4096
/*
 * Kernel stack size
 * 4KB
 */
#define K_STACK_SIZE_B 4096
/*
 * Kernel heap size
 * 4MB
 */
#define K_HEAP_SIZE_B 4194304

/**
 * Set the memory of some location
 * @param dest  the location
 * @param c     the memory to set
 * @param bytes the number of bytes to set
 */
void memset(void *dest, uint8_t c, uint64_t bytes);

/**
 * initialize the memory management sytem
 * @param phy_size the size of physical memory
 * @return the starting address of the kernel heap
 */
uint64_t init_mmu(uint64_t phy_size);

/**
 * Allocate a page
 */
void* palloc();

/**
 * Free an allocated page
 * @param addr the address of the page
 */
void pfree(void *addr);

#endif /*_MMU_MMU_H*/
