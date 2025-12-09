#include "uart.h"
#include "print.h"
#include <stdint.h>
#include <stdio.h>
#include "fpu.h"
#include "regs.h"
#include "mipsregs.h"

extern int main(void);

extern uint32_t _etext;
extern uint32_t _sdata;
extern uint32_t _edata;

extern uint32_t _sbss;
extern uint32_t _ebss;

extern uint32_t utlb_exception_end;
extern uint32_t utlb_exception;

extern uint32_t exception_end;
extern uint32_t exception;

void entrypoint(void) {
    // enable coprocessor 0 and 1
    // c0_setbits(CP0_STATUS, ST0_CU0 | ST0_CU1);
    // c0_clrbits(CP0_STATUS, ST0_IE | ST0_EXL | ST0_ERL | ST0_BEV);
    // c0_setbits(CP0_STATUS, ST0_EXL|ST0_ERL);
    // enable_fpu_hazard();

    // if (__cpu_has_fpu()) {
    // __enable_fpu(FPU_64BIT);
    // }

    // copy relocation segment
    uint32_t volatile *src = &_etext;
    uint32_t volatile *dst = &_sdata;
    uint32_t volatile size = &_edata - &_sdata;

    for (uint32_t i = 0; i < size; i++) {
        *dst++ = *src++;
    }

    // clear bss segment
    unsigned volatile long* ptr = &_sbss;
    unsigned volatile long* end = &_ebss;

    while (ptr < end) {
        *ptr++ = 0;
    }

    uart_attach(1);
    uart_set_mode(0, 115200, 0);
    kprintf("\nBooting...\n");

    // c0_clrbits(CP0_STATUS, ST0_BEV | ST0_EXL | ST0_ERL);
    // c0_setbits(CP0_STATUS, 0xff00); // enable all interrupts
    // c0_setbits(CP0_STATUS, ST0_IE);

    main();

    while(1) {}
}
