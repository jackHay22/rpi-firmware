/*
 * (C) Jack Hay, Apr 2021
 */

#include "console.h"
#include "display.h"
#include "../mmu/kheap.h"
#include "../kstdlib/kstdlib.h"
#include "../uart/debug.h"

//the size of the buffer
uint8_t SCREEN_ROWS = 0;
//the offset of the top line in the buffer
uint8_t TOP_LINE = 0;
//the console buffer
char **CONSOLE_BUFFER = NULL;

/**
 * Clear the screen and rerender lines
 */
void render_screen() {
  clear_screen();

  //render the lines in the buffer
  for (int i=0; i<SCREEN_ROWS; i++) {
    //draw the line on the screen
    draw_str(0,i * 10,CONSOLE_BUFFER[(TOP_LINE + i) % SCREEN_ROWS]);
  }
}

/**
 * Initialize the console
 * @return 0 on success, pos on error
 */
uint8_t init_console() {
  //initialize the display
  init_display();
  SCREEN_ROWS = DISPLAY_HEIGHT / 10;

  //init the console buffer
  CONSOLE_BUFFER = (char**) kmalloc(SCREEN_ROWS * sizeof(char*));

  if (!CONSOLE_BUFFER) {
    debug_err("console buffer allocation failed");
    return 1;
  }

  char *empty = "";

  //fill out the buffer to start
  for (int i=0; i<SCREEN_ROWS; i++) {
    CONSOLE_BUFFER[i] = (char*) kmalloc(strlen(empty) + 1);

    if (!CONSOLE_BUFFER[i]) {
      debug_err("console buffer line allocation failed");
      return 1;
    } else {
      memcpy(CONSOLE_BUFFER[i],empty,strlen(empty));
    }
  }

  render_screen();
  return 0;
}

/**
 * Write a string to the console
 * @param str the string to write
 */
void write_str(const char* str) {
  //expand the current line
  uint32_t exist_line_size = strlen(CONSOLE_BUFFER[TOP_LINE]) + 1;
  uint32_t new_str_size = strlen(str) + 1;

  char *prev = (char*) kmalloc(exist_line_size);
  memcpy(prev,CONSOLE_BUFFER[TOP_LINE],exist_line_size);

  //free the previous allocation
  kfree(CONSOLE_BUFFER[TOP_LINE]);

  //allocate space for entire string
  CONSOLE_BUFFER[TOP_LINE] = (char*) kmalloc(exist_line_size + new_str_size);
  memcpy(CONSOLE_BUFFER[TOP_LINE],prev,exist_line_size);
  memcpy(CONSOLE_BUFFER[TOP_LINE] + exist_line_size,str,new_str_size);
  kfree(prev);

  render_screen();
}

/**
 * Write a string and a newline
 * @param str the string
 */
void write_strln(char *str) {
  //free the previous line
  kfree(CONSOLE_BUFFER[TOP_LINE]);
  CONSOLE_BUFFER[TOP_LINE] = (char*) kmalloc(strlen(str)+1);
  memcpy(CONSOLE_BUFFER[TOP_LINE],str,strlen(str));

  //set the next offset
  TOP_LINE = (TOP_LINE + 1) % SCREEN_ROWS;

  render_screen();
}
