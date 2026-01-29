#include "stm32h755.h"

void delay(volatile int count) {
  while (count--) {
    __asm__("nop");
  }
}

void flush_cache_addr(unsigned int addr) {
  __asm__("dsb");     // Ensure all previous memory stores are finished
  SCB_DCCMVAC = addr; // Clean the cache line containing this address
  __asm__("dsb");     // Ensure the clean operation finished
  __asm__("isb");     // Flush the instruction pipeline
}
