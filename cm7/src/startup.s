.syntax unified
.cpu cortex-m7
.fpu fpv5-d16
.thumb

/* Symbols from linker script */
.extern _estack
.extern _sidata
.extern _sdata
.extern _edata
.extern _sbss
.extern _ebss

.extern main

/* FreeRTOS handlers */
.extern vPortSVCHandler
.extern xPortPendSVHandler
.extern xPortSysTickHandler

/* Vector table */
.section .isr_vector, "a", %progbits
.type g_pfnVectors, %object

g_pfnVectors:
  .word _estack
  .word Reset_Handler
  .word Default_Handler   /* NMI */
  .word Default_Handler   /* HardFault */
  .word Default_Handler   /* MemManage */
  .word Default_Handler   /* BusFault */
  .word Default_Handler   /* UsageFault */
  .word 0
  .word 0
  .word 0
  .word 0
  .word vPortSVCHandler
  .word Default_Handler   /* DebugMon */
  .word 0
  .word xPortPendSVHandler
  .word xPortSysTickHandler

/* Reset handler */
.section .text.Reset_Handler
.weak Reset_Handler
.type Reset_Handler, %function

Reset_Handler:
  /* Copy .data */
  ldr r0, =_sidata
  ldr r1, =_sdata
  ldr r2, =_edata
1:
  cmp r1, r2
  bcc 2f
  b 3f
2:
  ldr r3, [r0], #4
  str r3, [r1], #4
  b 1b

3:
  /* Zero .bss */
  ldr r0, =_sbss
  ldr r1, =_ebss
4:
  cmp r0, r1
  bcc 5f
  b 6f
5:
  movs r2, #0
  str r2, [r0], #4
  b 4b

6:
  bl main
  b .

/* Default handler */
.section .text.Default_Handler
Default_Handler:
  b .

