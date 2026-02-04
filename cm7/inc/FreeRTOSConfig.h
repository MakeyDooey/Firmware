#pragma once

#include <stdint.h>

/*-----------------------------------------------------------
 * Core configuration
 *----------------------------------------------------------*/

#define configUSE_PREEMPTION 1
#define configUSE_TIME_SLICING 1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1

#define configCPU_CLOCK_HZ (400000000UL) /* adjust if needed */
#define configTICK_RATE_HZ ((TickType_t)1000)
#define configMAX_PRIORITIES 5
#define configMINIMAL_STACK_SIZE 128
#define configTOTAL_HEAP_SIZE (64 * 1024)

#define configMAX_TASK_NAME_LEN 16
#define configUSE_16_BIT_TICKS 0
#define configIDLE_SHOULD_YIELD 1

/*-----------------------------------------------------------
 * Memory allocation
 *----------------------------------------------------------*/

#define configSUPPORT_STATIC_ALLOCATION 0
#define configSUPPORT_DYNAMIC_ALLOCATION 1

/*-----------------------------------------------------------
 * Hook functions
 *----------------------------------------------------------*/

#define configUSE_IDLE_HOOK 0
#define configUSE_TICK_HOOK 0
#define configCHECK_FOR_STACK_OVERFLOW 2
#define configUSE_MALLOC_FAILED_HOOK 1

/*-----------------------------------------------------------
 * Cortex-M specific
 *----------------------------------------------------------*/

#define configPRIO_BITS 4 /* STM32H7 uses 4 */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5

#define configKERNEL_INTERRUPT_PRIORITY                                                            \
    (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

#define configMAX_SYSCALL_INTERRUPT_PRIORITY                                                       \
    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

/*-----------------------------------------------------------
 * API inclusion
 *----------------------------------------------------------*/

#define INCLUDE_vTaskDelay 1
#define INCLUDE_vTaskSuspend 1
#define INCLUDE_xTaskGetSchedulerState 1
#define INCLUDE_uxTaskGetStackHighWaterMark 1
#define INCLUDE_xTaskGetCurrentTaskHandle 1

/*-----------------------------------------------------------
 * FreeRTOS+CLI
 *----------------------------------------------------------*/

#define configUSE_STATS_FORMATTING_FUNCTIONS 0
#define configCOMMAND_INT_MAX_OUTPUT_SIZE 256

/*-----------------------------------------------------------
 * Assert
 *----------------------------------------------------------*/

#define configASSERT(x)                                                                            \
    if ((x) == 0) {                                                                                \
        taskDISABLE_INTERRUPTS();                                                                  \
        for (;;)                                                                                   \
            ;                                                                                      \
    }
