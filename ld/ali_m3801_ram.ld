OUTPUT_ARCH(mips)

BASE_ADDR = 0x83000000;
STACK_ADDR = 0x81000000;
STACK_SIZE = 32K;
STACK_END = STACK_ADDR + STACK_SIZE;
HEAP_SIZE = 32K;
RAM_SIZE = 128M;
FLASH_SIZE = 4M;

MEMORY
{
  ram (rwx) : ORIGIN = 0x80000000, LENGTH = RAM_SIZE
  rom (rx)  : ORIGIN = BASE_ADDR, LENGTH = FLASH_SIZE
}

ENTRY(_start)

SECTIONS
{
    . = BASE_ADDR;

    .text :{
        _stext = .;
        *(.text .text.*)
        *(.rodata .rodata.*)
        *(.srodata .stodata.*)
        . = ALIGN(4);
    } > rom

   .data :
   {
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

    .stack STACK_ADDR (NOLOAD): {
        . = ALIGN(8);
        _sstack = .;
        . = . + STACK_SIZE;
        . = ALIGN(8);
        _estack = .;
    } > ram
}
