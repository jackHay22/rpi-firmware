/*
 * (C) Jack Hay, Apr 2021
 */

#ifndef _KSTDLIB_KSTDLIB_H
#define _KSTDLIB_KSTDLIB_H

#include <stdint.h>
#include <stddef.h>
#include <stdnoreturn.h>

/**
 * Memcopy
 * @param dest  destination to copy to
 * @param src   source to copy from
 * @param bytes bytes to copy
 */
void memcpy(void *dest, const void *src, uint32_t bytes);

/**
 * Get the length of a string
 * @param  str the null term string
 * @return     the length of the string
 */
uint32_t strlen(const char *str);

#endif /*_KSTDLIB_KSTDLIB_H*/
