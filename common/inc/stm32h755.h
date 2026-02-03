#ifndef STM32H755_H
#define STM32H755_H

/* Peripheral Base Addresses */
#define RCC_BASE 0x58024400
#define GPIOB_BASE 0x58020400
#define GPIOD_BASE 0x58020C00
#define USART3_BASE 0x40004800

/* RCC Registers */
#define RCC_AHB4ENR (*(volatile unsigned int *)(RCC_BASE + 0xE0))
#define RCC_APB1LENR (*(volatile unsigned int *)(RCC_BASE + 0xE8))
#define RCC_GCR (*(volatile unsigned int *)(RCC_BASE + 0x00))

#define RCC_AHB4ENR_GPIOBEN (1 << 1)
#define RCC_AHB4ENR_GPIODEN (1 << 3)
#define RCC_APB1LENR_USART3EN (1 << 18)
#define RCC_GCR_BOOT_C4 (1 << 0)

/* GPIO Registers */
#define GPIOB_MODER (*(volatile unsigned int *)(GPIOB_BASE + 0x00))
#define GPIOB_ODR (*(volatile unsigned int *)(GPIOB_BASE + 0x14))
#define GPIOD_MODER (*(volatile unsigned int *)(GPIOD_BASE + 0x00))
#define GPIOD_AFRH (*(volatile unsigned int *)(GPIOD_BASE + 0x24))

#define GPIO_MODER_OUT (1)
#define GPIO_MODER_AF (2)

/* USART3 Registers */
#define USART3_CR1 (*(volatile unsigned int *)(USART3_BASE + 0x00))
#define USART3_BRR (*(volatile unsigned int *)(USART3_BASE + 0x0C))
#define USART3_ISR (*(volatile unsigned int *)(USART3_BASE + 0x1C))
#define USART3_TDR (*(volatile unsigned int *)(USART3_BASE + 0x28))

#define USART_CR1_UE (1 << 0)
#define USART_CR1_RE (1 << 2)
#define USART_CR1_TE (1 << 3)
#define USART_ISR_TXE (1 << 7)

#endif
