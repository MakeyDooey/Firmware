#include "common.h"

int main(void) {
  uart_init();
  uart_puts("--- CM7 System Master Online ---\n");

  /* Initialize Yellow LED (PB0) */
  RCC_AHB4ENR |= RCC_AHB4ENR_GPIOBEN;
  GPIOB_MODER &= ~(0x3 << 0);
  GPIOB_MODER |= (GPIO_MODER_OUT << 0);

  /* Wake up the CM4 core */
  uart_puts("CM7: Releasing CM4 from reset...\n");
  RCC_GCR |= RCC_GCR_BOOT_C4;

  while (1) {
    GPIOB_ODR ^= (1 << 0); // Toggle Yellow
    uart_puts("CM7 Pulse...\n");
    delay(5000000);
  }
}
