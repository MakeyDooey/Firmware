void delay(volatile int count) {
  while (count--) {
    __asm__("nop");
  }
}
