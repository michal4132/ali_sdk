OUTPUT_ARCH(mips)

BASE_ADDR = 0xAFC00000;
STACK_ADDR = 0x82000000;
STACK_SIZE = 32K;
STACK_END = STACK_ADDR + STACK_SIZE;
HEAP_SIZE = 32K;
RAM_SIZE = 128M;
FLASH_SIZE = 4M;
VECTOR_ADDR = 0x80000000;

MEMORY
{
  ram (rwx) : ORIGIN = 0x81000000, LENGTH = RAM_SIZE
  rom (rx)  : ORIGIN = 0xAFC00000, LENGTH = FLASH_SIZE
}

ENTRY(_start)

SECTIONS
{
    . = BASE_ADDR;

    .text : {
        _stext = .;
        *(.text .text.*)
        *(.rodata .rodata.*)
        *(.srodata .stodata.*)
        . = ALIGN(4);
    } > rom

    .vector : {
        _svector = VECTOR_ADDR;
        *(.vector .vector.*)
        _evector = .;
    } > rom

    .data : {
        _sdata = .;
        *(.data .data.*)
        *(.sdata .sdata.*)
        . = ALIGN(4);
        _edata = .;
    } > ram AT> rom

    _etext = LOADADDR(.data);

    .bss (NOLOAD) : {
        _sbss = .;
        __bss_start__ = _sbss;
        *(.bss .bss.*)
        *(COMMON)
        _ebss = .;
        __bss_end__ = _ebss;
        . = ALIGN(4);
        _end = .;
    } > ram

    .heap : {
        _heap_start = .;
        . = ALIGN(4);
        . = . + HEAP_SIZE;
        _heap_end = .;
    } > ram

    .stack STACK_ADDR (NOLOAD) : {
        . = ALIGN(8);
        _sstack = .;
        . = . + STACK_SIZE;
        . = ALIGN(8);
        _estack = .;
    } > ram
}
