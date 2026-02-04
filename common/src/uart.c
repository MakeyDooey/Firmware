#include "uart.h"
#include "stm32h755.h"

void uart_init(void)
{
    // 1. Enable Clocks
    RCC_AHB4ENR |= RCC_AHB4ENR_GPIODEN;
    RCC_APB1LENR |= RCC_APB1LENR_USART3EN;

    // 2. Clear and Set Mode for PD8 and PD9 (Bits 16-19)
    GPIOD_MODER &= ~(0xF << 16);
    GPIOD_MODER |= (GPIO_MODER_AF << 16) | (GPIO_MODER_AF << 18);

    // 3. AFRH: PD8 is AF7 (bits 0-3), PD9 is AF7 (bits 4-7)
    GPIOD_AFRH &= ~(0xFF << 0);
    GPIOD_AFRH |= (0x77 << 0);

    // 4. Baud rate (Assuming 64MHz HSI is the source)
    USART3_BRR = 556;

    // 5. Enable UE, TE, and RE
    USART3_CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

void uart_putc(char c)
{
    while (!(USART3_ISR & USART_ISR_TXE_TXFNF))
        ;
    USART3_TDR = c;
}

void uart_puts(const char *s)
{
    while (*s) {
        if (*s == '\n')
            uart_putc('\r');
        uart_putc(*s++);
    }
}

int uart_getc(void)
{
    /* RXNE = Receive data register not empty */
    if (USART3_ISR & USART_ISR_RXNE_RXFNE) {
        return (int)(USART3_RDR & 0xFF);
    }
    return -1;
}
