#include <uart.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regs.h>
#include <mipsregs.h>
#include <print.h>
#include <gpio.h>

// sample user program

extern uint32_t _sstack;
extern uint32_t _estack;

extern uint32_t _heap_start;
extern uint32_t _heap_end;

void print_addr(void) {
    kprintf("stack end: %x\n\r", &_sstack);
    kprintf("stack start: %x\n\r", &_estack);

    kprintf("heap start: %x\n\r", &_heap_start);
    kprintf("heap end: %x\n\r", &_heap_end);
}

int readline(char *buf, int len) {
    uint8_t ch;
    int i = 0;
    while (i + 1 < len) {
        ch = uart_read_char(0);

        if (ch == '\n' || ch == '\r') {
            break;
        }
        uart_write_char(0, ch);
        buf[i] = ch;
        i++;
    }
    uart_write_char(0, '\n');
    buf[i] = '\0';
    return i;
}

#define LINE_BUFFER_SIZE 100
static char line[LINE_BUFFER_SIZE];

typedef struct Cmd {
    char *name;
    void (*f)(void);
} Cmd;

// ping
void cmd_f(void) {
    kprintf("cmd ok\n\r");
}

// write data to ram
void write_f(void) {
    kprintf("enter size: ");
    fflush(stdout);
    readline(line, 100);
    uint32_t len = strtoul(line, NULL, 0);
    kprintf("waiting for data: %d\n\r", len);
    char volatile *target = (char *)0x83000000;
    for (uint32_t i = 0; i < len; i++) {
        target[i] = uart_read_char(0);
    }
    kprintf("written\n\r");
}

// execute program at address
void jump_f(void) {
    kprintf("enter address: ");
    fflush(stdout);
    readline(line, 100);
    uint32_t addr = strtoul(line, NULL, 0);
    kprintf("addr: %a correct [y/N] ", addr);
    fflush(stdout);
    int len = readline(line, 100);
    if (line[0] == 'y' && len == 1) {
        kprintf("jump\n\r");
        exec(addr);
    }
}

// dump data from memory to uart
void dump_f(void) {
    kprintf("enter address: ");
    fflush(stdout);
    readline(line, 100);
    uint32_t addr = strtoul(line, NULL, 0);

    kprintf("enter length: ");
    fflush(stdout);
    readline(line, 100);
    uint32_t len = strtoul(line, NULL, 0);

    kprintf("addr: %a, len: %u correct [y/N] ", addr, len);
    fflush(stdout);

    int confirm_len = readline(line, 100);
    if (line[0] == 'y' && confirm_len == 1) {
        kprintf("dump\n\r");
        uint32_t volatile *target = (uint32_t *)addr;
        for (uint32_t i = 0; i < len; i++) {
            kprintf("%a: %a\n\r", addr + (4*i), target[i]);
        }
    }
}

// TODO(michal4132): fix
// interrupt tester
void test_f(void) {
    uint32_t addr = 0x81000000;
    kprintf("%u\n\r", addr);
    kprintf("%a\n\r", addr);
    c0_clrbits(CP0_STATUS, ST0_BEV | ST0_EXL | ST0_ERL);
    c0_setbits(CP0_STATUS, 0xff00); // enable all interrupts
    c0_setbits(CP0_STATUS, ST0_IE);
    kprintf("OK\n\r");

}

void run_f(void) {
    kprintf("load from ram\n\r");
    exec(0x83000000);
}

Cmd cmd[] = {
    {"cmd", cmd_f},
    {"write", write_f},
    {"jump", jump_f},
    {"run", run_f},
    {"dump", dump_f},
    {"test", test_f}
};

int main(void) {

    kprintf("Main function\n\r");

    print_addr();

    int cause = read_32bit_cause_register();
    kprintf("cause: %x\n\r", cause);

    double x = 1.23;
    double y = 1.25;

    kprintf("x: %f y: %f\n\r", x, y);

    for (int i = 0; i < 10; i++) {
        x += y + 1.2;
    }

    kprintf("result: %f\n\r", x);

    cause = read_32bit_cause_register();
    kprintf("cause: %x\n\r", cause);

	if(1) {
		// GPIO test
		// My two buttons on board are working 
		// (it has no i2c-like display, no display at all)
		// I also test it by randomly shorting random GPIOs to ground,
		// I can cleary see that at least some of them react
		// all in reproducible manner
		for(int i = 0; i < GPIO_PORT_MAX; i++) {
			gpio_direction_input(i);
		}
		while(1) {
			for(int i = 0; i < GPIO_PORT_MAX; i+=8) {
				int x = 0;
				for(int j = 0; j < 8; j++) {
					int val = gpio_get_value(i + j);
					if(val) x |= (1 << j);
				}
				kprintf("%x ", x);
			}
			kprintf("\n\r");
		}
	}

    // command loop
    while (1) {
        readline(line, 100);
        for (uint8_t i = 0; i < sizeof(cmd)/sizeof(cmd[0]); i++) {
            if (strncmp(line, cmd[i].name, 100) == 0) {
                cmd[i].f();
                break;
            }
        }
    }

    return 0;
}
