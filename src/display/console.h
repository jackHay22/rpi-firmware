/*
 * (C) Jack Hay, Apr 2021
 */

#ifndef _DISPLAY_CONSOLE_H
#define _DISPLAY_CONSOLE_H

#include <stdint.h>
#include <stddef.h>

/**
 * Initialize the console
 * @return 0 on success, pos on error
 */
uint8_t init_console();

/**
 * Write a string to the console
 * @param str the string to write
 */
void write_str(const char* str);

/**
 * Write a string and a newline
 * @param str the string
 */
void write_strln(char *str);

#endif /*_DISPLAY_CONSOLE_H*/
