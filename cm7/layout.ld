/* cm7/layout.ld */

MEMORY
{
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 1024K
    RAM (rwx)  : ORIGIN = 0x24000000, LENGTH = 512K
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

    /* The Stack Pointer MUST be 8-byte aligned */
    _estack = ORIGIN(RAM) + LENGTH(RAM);
}
