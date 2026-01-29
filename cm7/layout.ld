
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
        
        /* Force 32-byte alignment at the end of the section */
        . = ALIGN(32);
    } > FLASH

    _estack = ORIGIN(RAM) + LENGTH(RAM);
}
