#include "common.h"

void uart_init(void) {
  /* 1. Enable Clocks for GPIOD and USART3 */
  RCC_AHB4ENR |= RCC_AHB4ENR_GPIODEN;
  RCC_APB1LENR |= RCC_APB1LENR_USART3EN;

  /* 2. PD8 (TX) and PD9 (RX) as Alternate Function */
  GPIOD_MODER &= ~((0x3 << 16) | (0x3 << 18));
  GPIOD_MODER |= ((GPIO_MODER_AF << 16) | (GPIO_MODER_AF << 18));

  /* 3. Map to AF7 (USART3) in AFR High register */
  GPIOD_AFRH &= ~((0xF << 0) | (0xF << 4));
  GPIOD_AFRH |= ((0x7 << 0) | (0x7 << 4));

  /* 4. Baud Rate: 115200 @ 64MHz HSI. BRR = 64000000 / 115200 ≈ 556 */
  USART3_BRR = 556;

  /* 5. Enable Peripheral */
  USART3_CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

void uart_putc(char c) {
  while (!(USART3_ISR & USART_ISR_TXE))
    ;
  USART3_TDR = c;
}

void uart_puts(const char *s) {
  while (*s) {
    if (*s == '\n')
      uart_putc('\r');
    uart_putc(*s++);
  }
}
