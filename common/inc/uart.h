#ifndef UART_H
#define UART_H

/**
 * @brief Initializes USART3 on PD8 (TX) and PD9 (RX).
 * Configures for 115200 Baud, 8N1, based on 64MHz HSI clock.
 */
void uart_init(void);

/**
 * @brief Transmits a single character.
 * Blocks until the Transmit Data Register is empty.
 * @param c The character to send.
 */
void uart_putc(char c);

/**
 * @brief Transmits a null-terminated string.
 * Automatically converts '\n' to '\r\n' for terminal compatibility.
 * @param s Pointer to the string.
 */
void uart_puts(const char *s);

/**
 * @brief Receives a single character.
 * Blocks until a character is available in the Read Data Register.
 * @return The received character.
 */
char uart_getc(void);

#endif /* UART_H */
