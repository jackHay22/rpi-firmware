/*
 * (C) Jack Hay, Apr 2021
 */

#ifndef _UART_DEBUG_H
#define _UART_DEBUG_H

#include <stdint.h>
#include <stddef.h>

//error allocating a physical page
#define ERRNO_PALLOC 1
//error calling kmalloc (space)
#define ERRNO_KMALLOC 2

/**
 * Log a message
 * @param msg the message to log
 */
void debug_log(const char* msg);

/**
 * Log an error
 * @param msg the error message
 */
void debug_err(const char* msg);

/**
 * Debug some value
 * @param name the identifier
 * @param val  the value itself
 */
void debug_val(const char* name, uint64_t val);

/**
 * Set the errno
 * @param code exit code
 */
void set_errno(uint8_t code);

#endif /*_UART_DEBUG_H*/
