/* mps2.ld — Fixed Linker script for QEMU MPS2-AN385 Cortex-M3
 * Flash: 0x00000000, 4MB
 * SRAM:  0x20000000, 4MB
 */

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 4M
    SRAM  (rwx) : ORIGIN = 0x20000000, LENGTH = 4M
}

/* Stack at very top of SRAM */
_estack = ORIGIN(SRAM) + LENGTH(SRAM);   /* 0x20400000 */

SECTIONS
{
    /* Code + read-only data -> Flash */
    .text :
    {
        KEEP(*(.isr_vector))
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
        _etext = .;
    } > FLASH

    /* Initialized data: stored in Flash, runs in SRAM */
    _sidata = LOADADDR(.data);
    .data :
    {
        _sdata = .;
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > SRAM AT > FLASH

    /* Uninitialized data: zeroed in SRAM */
    .bss :
    {
        _sbss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > SRAM

    /* Heap starts here - used by _sbrk */
    . = ALIGN(8);
    _end = .;
}
