/*
 * (C) Jack Hay, Apr 2021
 */

#include "debug.h"
#include "uart.h"
#include "../kstdlib/kstdlib.h"

/*
 * Exit code on failure
 */
uint8_t ERRNO = 0;

/**
 * Log a message
 * @param msg the message to log
 */
void debug_log(const char* msg) {
  uart_puts("[LOG] ");
  uart_puts(msg);
  uart_puts("\n");
}

/**
 * Convert an int to a string
 * (Base 10)
 * @param num  the number to convert
 * @param buff the buffer
 */
void itoa(uint64_t num, char *buff) {
  uint32_t i = 20;

  char temp[22];
  temp[21] = 0;
  uint64_t rem;

  while (1) {
    rem = num % 10;
    temp[i] = rem + '0';
    if (num > 10) {
      num /= 10;
      i--;
    } else {
      break;
    }
  }

  //copy into buffer
  memcpy(buff,temp + i, 22 - i);
}


/**
 * Debug some value
 * @param name the identifier
 * @param val  the value itself
 */
void debug_val(const char* name, uint64_t val) {
  uart_puts("[LOG] ");
  uart_puts(name);
  uart_puts(": ");
  char *buf = "00000000000000000000\0";
  itoa(val,buf);
  uart_puts(buf);
  uart_puts("\n");
}

/**
 * Convert the errno to a string
 * @param a the string to set (length 3 for uint8_t)
 */
void errno_toa(char* a) {
  int curr = ERRNO;
  while (curr > 0) {
    if (curr >= 100) {
      a[0] = (char) (curr / 100) + '0';
      curr = curr % 100;
    } else if (curr >= 10) {
      a[1] = (char) (curr / 10) + '0';
      curr = curr % 10;
    } else {
      a[2] = (char) curr + '0';
      curr = 0;
    }
  }
}

/**
 * Log an error
 * @param msg the error message
 */
void debug_err(const char* msg) {
  uart_puts("[ERR] ");
  char *errno = "000\0";
  errno_toa(errno);
  uart_puts(errno);
  uart_puts(": ");
  uart_puts(msg);
  uart_puts("\n");
}

/**
 * Set the errno
 * @param code exit code
 */
void set_errno(uint8_t code) {
  ERRNO = code;
}
