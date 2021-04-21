/*
 * (C) Jack Hay, Apr 2021
 */

#ifndef _DISPLAY_DISPLAY_H
#define _DISPLAY_DISPLAY_H

#include <stdint.h>
#include <stddef.h>

#define DISPLAY_WIDTH 512
#define DISPLAY_HEIGHT 384

/**
 * Initialize the display
 */
void init_display();

/**
 * Clear the screen
 */
void clear_screen();

/**
 * Write a message to the console
 * @param x position x
 * @param y position y
 * @param msg the message to write
 */
void draw_str(int x, int y, char *msg);

#endif /*_DISPLAY_DISPLAY_H*/
