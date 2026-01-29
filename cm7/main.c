
#include "../common/stm32h755.h"

void main(void) {
  RCC_AHB4ENR |= (1 << 1); // Enable GPIOB Clock

  for (int ii = 0; ii < 100; ii++) {
    __asm__("nop");
  }

  SHARED_VAL = 10;

  GPIOB_MODER &= ~(0x3); // Setup PB0 (Yellow)
  GPIOB_MODER |= (0x1);

  delay(100000);

  RCC_GCR |= (1 << 0); // Wake up Cortex-M4

  while (1) {
    GPIOB_ODR ^= (1 << 0);
    delay(2000000);
  }
}

void Reset_Handler(void) { main(); }
__attribute__((section(".isr_vector"))) unsigned int *my_vectors[] = {
    (unsigned int *)0x24080000, (unsigned int *)Reset_Handler};
