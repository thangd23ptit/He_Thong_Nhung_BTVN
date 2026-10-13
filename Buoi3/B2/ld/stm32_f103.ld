/*
 * stm32_f103.ld
 * Bare-metal linker script for STM32F103C8T6 / STM32F103x8
 * Flash: 64 KB @ 0x08000000
 * RAM  : 20 KB @ 0x20000000
 */

ENTRY(Reset_Handler)

_estack = ORIGIN(RAM) + LENGTH(RAM);

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 20K
}

SECTIONS
{
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH

    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.rodata)
        *(.rodata*)
        *(.glue_7)
        *(.glue_7t)
        KEEP(*(.init))
        KEEP(*(.fini))
        . = ALIGN(4);
        _etext = .;
    } > FLASH

    .ARM.exidx :
    {
        . = ALIGN(4);
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
        . = ALIGN(4);
    } > FLASH

    _sidata = LOADADDR(.data);

    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        __data_start__ = _sdata;

        *(.data)
        *(.data*)

        . = ALIGN(4);
        _edata = .;
        __data_end__ = _edata;
    } > RAM AT > FLASH

    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        _sbss = .;
        __bss_start__ = _sbss;

        *(.bss)
        *(.bss*)
        *(COMMON)

        . = ALIGN(4);
        _ebss = .;
        __bss_end__ = _ebss;
    } > RAM

    . = ALIGN(4);
    _end = .;
    end = .;

    ASSERT(_ebss <= ORIGIN(RAM) + LENGTH(RAM),
           "ERROR: RAM overflow")

    ASSERT(_etext + SIZEOF(.data) <= ORIGIN(FLASH) + LENGTH(FLASH),
           "ERROR: FLASH overflow")

    /DISCARD/ :
    {
        *(.note.GNU-stack)
        *(.comment)
        *(.eh_frame)
    }
}