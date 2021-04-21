/*
 * (C) Jack Hay, Apr 2021
 */

#include "kstdlib.h"

/**
 * Memcopy
 * @param dest  destination to copy to
 * @param src   source to copy from
 * @param bytes bytes to copy
 */
void memcpy(void *dest, const void *src, uint32_t bytes) {
  char *d = (char*)dest;
  const char * s = (const char*)src;
  for (uint32_t i=0; i<bytes; i++) {
    d[i] = s[i];
  }
}

/**
 * Get the length of a string
 * @param  str the null term string
 * @return     the length of the string
 */
uint32_t strlen(const char *str) {
  uint32_t size = 0;
  const char *c = str;
  while (*c != 0) {
    c++;
    size += sizeof(char);
  }
  return size;
}
