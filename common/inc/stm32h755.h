#ifndef STM32H755_H
#define STM32H755_H

/* ================= Peripheral Base Addresses ================= */

#define RCC_BASE 0x58024400UL
#define GPIOB_BASE 0x58020400UL
#define GPIOD_BASE 0x58020C00UL
#define USART3_BASE 0x40004800UL

/* ================= RCC Registers ================= */

#define RCC_AHB4ENR (*(volatile unsigned int *)(RCC_BASE + 0xE0))
#define RCC_APB1LENR (*(volatile unsigned int *)(RCC_BASE + 0xE8))
#define RCC_GCR (*(volatile unsigned int *)(RCC_BASE + 0x00))

/* RCC bits */
#define RCC_AHB4ENR_GPIOBEN (1U << 1)
#define RCC_AHB4ENR_GPIODEN (1U << 3)
#define RCC_APB1LENR_USART3EN (1U << 18)
#define RCC_GCR_BOOT_C4 (1U << 0)

/* ================= GPIO Registers ================= */

#define GPIOB_MODER (*(volatile unsigned int *)(GPIOB_BASE + 0x00))
#define GPIOB_ODR (*(volatile unsigned int *)(GPIOB_BASE + 0x14))

#define GPIOD_MODER (*(volatile unsigned int *)(GPIOD_BASE + 0x00))
#define GPIOD_AFRL (*(volatile unsigned int *)(GPIOD_BASE + 0x20))
#define GPIOD_AFRH (*(volatile unsigned int *)(GPIOD_BASE + 0x24))

/* GPIO modes */
#define GPIO_MODER_IN 0
#define GPIO_MODER_OUT 1
#define GPIO_MODER_AF 2
#define GPIO_MODER_AN 3

/* ================= USART3 Registers ================= */

#define USART3_CR1 (*(volatile unsigned int *)(USART3_BASE + 0x00))
#define USART3_CR2 (*(volatile unsigned int *)(USART3_BASE + 0x04))
#define USART3_CR3 (*(volatile unsigned int *)(USART3_BASE + 0x08))
#define USART3_BRR (*(volatile unsigned int *)(USART3_BASE + 0x0C))
#define USART3_ISR (*(volatile unsigned int *)(USART3_BASE + 0x1C))
#define USART3_ICR (*(volatile unsigned int *)(USART3_BASE + 0x20))
#define USART3_RDR (*(volatile unsigned int *)(USART3_BASE + 0x24))
#define USART3_TDR (*(volatile unsigned int *)(USART3_BASE + 0x28))

/* ================= USART CR1 bits ================= */

#define USART_CR1_UE (1U << 0)
#define USART_CR1_RE (1U << 2)
#define USART_CR1_TE (1U << 3)

/* ================= USART ISR bits (STM32H7) ================= */

/* TX */
#define USART_ISR_TXE_TXFNF (1U << 7) /* TX FIFO not full */

/* RX */
#define USART_ISR_RXNE_RXFNE (1U << 5) /* RX FIFO not empty */

/* Errors (optional but useful later) */
#define USART_ISR_ORE (1U << 3)
#define USART_ISR_NE (1U << 2)
#define USART_ISR_FE (1U << 1)

#endif /* STM32H755_H */
