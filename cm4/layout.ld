/* cm4/layout.ld */

MEMORY
{
    /* Bank 2: 1MB */
    FLASH (rx) : ORIGIN = 0x08100000, LENGTH = 1024K
    /* SRAM4: 64KB (Accessible by D3 domain) */
    RAM (rwx)  : ORIGIN = 0x38000000, LENGTH = 64K
}

SECTIONS
{
    .text :
    {
        KEEP(*(.isr_vector))
        *(.text*)
        *(.rodata*)
        . = ALIGN(32);
    } > FLASH

    /* Stack pointer for M4 */
    _estack = ORIGIN(RAM) + LENGTH(RAM);
}
