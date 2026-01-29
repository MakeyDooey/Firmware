#ifndef STM32H755_H
#define STM32H755_H

// Clock Control
#define RCC_BASE 0x58024400
#define RCC_AHB4ENR (*(volatile unsigned int *)(RCC_BASE + 0xE0))
#define RCC_APB1LENR (*(volatile unsigned int *)(RCC_BASE + 0xCC))
#define RCC_GCR (*(volatile unsigned int *)(RCC_BASE + 0x00))

// GPIO Port B
#define GPIOB_BASE 0x58020400
#define GPIOB_MODER (*(volatile unsigned int *)(GPIOB_BASE + 0x00))
#define GPIOB_ODR (*(volatile unsigned int *)(GPIOB_BASE + 0x14))

// Utility Function Prototype
void delay(volatile int count);

#endif
