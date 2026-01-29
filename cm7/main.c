
/* ~/Firmware/cm7/main.c */

// User Defined Constant
#define BUSYWAIT 4000000

// Register Addresses
#define RCC_AHB4ENR (*(volatile unsigned int *)0x580244E0)
#define GPIOB_MODER (*(volatile unsigned int *)0x58020400)
#define GPIOB_ODR (*(volatile unsigned int *)0x58020414)

void delay(volatile int count) {
  while (count--) {
    __asm__("nop");
  }
}

int main(void) {
  // 1. Enable clock for GPIOB (Bit 1)
  RCC_AHB4ENR |= (1 << 1);

  // 2. Set PB0 to Output mode (01 in bits 1:0)
  GPIOB_MODER &= ~(0x3);
  GPIOB_MODER |= (0x1);

  while (1) {
    GPIOB_ODR ^= 0x1; // Toggle Pin 0
    delay(BUSYWAIT);  // Simple busy-wait
  }
}

// Minimal Startup Logic
void Reset_Handler(void) { main(); }

// Define the Vector Table
extern unsigned int _estack;
__attribute__((section(".isr_vector"))) unsigned int *my_vectors[] = {
    (unsigned int *)&_estack,     // 0: Initial Stack Pointer
    (unsigned int *)Reset_Handler // 1: Reset Vector (where code starts)
};
