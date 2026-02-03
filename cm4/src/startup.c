#include "common.h"

extern unsigned int _sidata, _sdata, _edata, _sbss, _ebss, _estack;
extern int main(void);

void Reset_Handler(void) {
  /* Copy .data from Flash to RAM */
  unsigned int *src = &_sidata;
  unsigned int *dst = &_sdata;
  while (dst < &_edata)
    *dst++ = *src++;

  /* Zero .bss */
  dst = &_sbss;
  while (dst < &_ebss)
    *dst++ = 0;

  main();
  while (1)
    ;
}

__attribute__((section(".isr_vector"))) unsigned int *vectors[] = {
    (unsigned int *)&_estack, (unsigned int *)Reset_Handler};
