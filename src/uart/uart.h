/*
 * (C) Jack Hay, Apr 2021
 */

#ifndef _UART_UART_H
#define _UART_UART_H

/**
 * Initialize uart
 */
void init_uart();

/**
 * Send a character
 * @param c the character to send
 */
void uart_putc(unsigned char c);

/**
 * Get a character
 * @return the received character
 */
unsigned char uart_getc();

/**
 * Put a string
 * @param str the string
 */
void uart_puts(const char* str);

#endif /*_UART_UART_H*/
