#include "../common/stm32h755.h"

void main(void) {
  // Clock already enabled by CM7
  GPIOB_MODER &= ~(0x3 << 28); // Setup PB14 (Red)
  GPIOB_MODER |= (0x1 << 28);

  while (1) {
    unsigned int speed_multiplier = SHARED_VAL + 1;
    GPIOB_ODR ^= (1 << 14);
    delay(2000000 * speed_multiplier);
  }
}

void Reset_Handler(void) { main(); }
__attribute__((section(".isr_vector"))) unsigned int *my_vectors[] = {
    (unsigned int *)0x38010000, (unsigned int *)Reset_Handler};
