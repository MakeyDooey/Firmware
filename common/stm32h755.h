#ifndef STM32H755_H
#define STM32H755_H

/* Hardware Base Addresses */
#define RCC_BASE 0x58024400
#define GPIOB_BASE 0x58020400
#define MPU_BASE 0xE000ED90

/* RCC & GPIO Registers */
#define RCC_AHB4ENR (*(volatile unsigned int *)(RCC_BASE + 0xE0))
#define RCC_GCR (*(volatile unsigned int *)(RCC_BASE + 0x00))
#define GPIOB_MODER (*(volatile unsigned int *)(GPIOB_BASE + 0x00))
#define GPIOB_ODR (*(volatile unsigned int *)(GPIOB_BASE + 0x14))

/* Shared Memory (First 256 bytes of SRAM4) */
#define SHARED_VAL_ADDR 0x38000000
#define SHARED_VAL (*(volatile unsigned int *)SHARED_VAL_ADDR)

/* MPU Registers (Cortex-M7) */
#define MPU_CTRL (*(volatile unsigned int *)(MPU_BASE + 0x04))
#define MPU_RNR (*(volatile unsigned int *)(MPU_BASE + 0x08))
#define MPU_RBAR (*(volatile unsigned int *)(MPU_BASE + 0x0C))
#define MPU_RASR (*(volatile unsigned int *)(MPU_BASE + 0x10))

// SCB_DCCMVAC: Data Cache Clean by MVA to PoC (Point of Coherency)
#define SCB_DCCMVAC (*(volatile unsigned int *)0xE000EF68)

/* Common Functions */
void delay(volatile int count);
void flush_cache_addr(unsigned int addr);

#endif
