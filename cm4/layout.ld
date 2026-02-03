MEMORY
{
    /* CM4 can run from Flash, but let's use a specific offset or RAM for this example */
    FLASH (rx) : ORIGIN = 0x08100000, LENGTH = 1024K 
    RAM (rwx)  : ORIGIN = 0x10000000, LENGTH = 288K  /* DTCM for CM4 */
}

SECTIONS
{
    .text :
    {
        KEEP(*(.isr_vector))
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
    } > FLASH

    _sidata = LOADADDR(.data);

    .data :
    {
        _sdata = .;
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM AT > FLASH

    .bss :
    {
        _sbss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM

    _estack = ORIGIN(RAM) + LENGTH(RAM);
}
