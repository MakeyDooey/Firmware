#include "common.h"

int main(void) {
  // Clock already enabled by CM7
  GPIOB_MODER &= ~(0x3 << 28); // Setup PB14 (Red)
  GPIOB_MODER |= (0x1 << 28);

  while (1) {
    __asm__("dsb");

    unsigned int val = SHARED_VAL;
    unsigned int speed = (val > 0) ? val : 1;

    GPIOB_ODR ^= (1 << 14);
    delay(200000 * speed);
  }

  return 0;
}

void Reset_Handler(void) { main(); }
__attribute__((section(".isr_vector"))) unsigned int *my_vectors[] = {
    (unsigned int *)0x38010000, (unsigned int *)Reset_Handler};
