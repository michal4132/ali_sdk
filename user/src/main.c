#include <uart.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regs.h>
#include <mipsregs.h>
#include <print.h>
#include <gpio.h>
#include <chip.h>
// 
/*
TODO TOP IDEA:
https://github.com/bnister/PDK_GoBian/blob/e5da0239b43229b8e5212508d79e962b8bbaece5/pdk/linux/kernel/alidrivers/modules/aliarch/mips/ali/m36/m36_irq.c#L72
*/



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


/*===========================================================================
 * Volatile counter - incremented by timer interrupt
 *===========================================================================*/
volatile uint32_t tick_count = 0;

/*===========================================================================
 * CP0 Register Access Macros (MIPS32 Vol 3, Section 1)
 *===========================================================================*/
#define MFC0(dst, reg, sel)                                     \
    __asm__ volatile("mfc0 %0, $" #reg ", " #sel : "=r"(dst))

#define MTC0(src, reg, sel)                                     \
    __asm__ volatile("mtc0 %0, $" #reg ", " #sel :: "r"(src));  \
    __asm__ volatile("ehb")

/* Status Register bits (MIPS32 Vol 3, Section 8.25) */
#define ST_IE           (1 << 0)    /* Global Interrupt Enable */
#define ST_EXL          (1 << 1)    /* Exception Level */
#define ST_ERL          (1 << 2)    /* Error Level */
#define ST_IM0          (1 << 8)    /* Interrupt Mask 0 (SW0) */
#define ST_IM1          (1 << 9)    /* Interrupt Mask 1 (SW1) */
#define ST_IM2          (1 << 10)   /* Interrupt Mask 2 (HW0) */
#define ST_IM3          (1 << 11)   /* Interrupt Mask 3 (HW1) */
#define ST_IM4          (1 << 12)   /* Interrupt Mask 4 (HW2) */
#define ST_IM5          (1 << 13)   /* Interrupt Mask 5 (HW3) */
#define ST_IM6          (1 << 14)   /* Interrupt Mask 6 (HW4) */
#define ST_IM7          (1 << 15)   /* Interrupt Mask 7 (HW5/Timer) */
#define ST_BEV          (1 << 22)   /* Bootstrap Exception Vector */

/* Timer interval (adjust based on CPU frequency) */
#define TIMER_INTERVAL  10000000    /* ~100ms at 100MHz */

/*===========================================================================
 * Timer Interrupt Handler (called from assembly)
 *===========================================================================*/
void timer_interrupt_handler(void)
{
    uint32_t count;
    
    /* Increment tick counter */
    tick_count++;
    
    /* 
     * Clear timer interrupt by writing new Compare value
     * (MIPS32 Vol 3, Section 8.6)
     * Timer interrupt is cleared when Compare is written
     */
    MFC0(count, 9, 0);                      /* Read Count */
    MTC0(count + TIMER_INTERVAL, 11, 0);    /* Write Compare */
}

/*===========================================================================
 * Enable/Disable Interrupts
 *===========================================================================*/
static inline void enable_interrupts(void)
{
    uint32_t status;
    MFC0(status, 12, 0);
    status |= ST_IE;
    MTC0(status, 12, 0);
}

static inline void disable_interrupts(void)
{
    uint32_t status;
    MFC0(status, 12, 0);
    status &= ~ST_IE;
    MTC0(status, 12, 0);
}

/*===========================================================================
 * Setup Timer Interrupt
 *===========================================================================*/
static void setup_timer(void)
{
    uint32_t status;
    
    /* Clear Count register (MIPS32 Vol 3, Section 8.5) */
    MTC0(0, 9, 0);
    
    /* Set Compare for first interrupt (MIPS32 Vol 3, Section 8.6) */
    MTC0(TIMER_INTERVAL, 11, 0);
    
    /* 
     * Configure Status register (MIPS32 Vol 3, Section 8.25):
     * - Clear BEV (use RAM exception vectors at 0x80000000)
     * - Clear EXL, ERL (normal operation)
     * - Enable IM7 (timer interrupt = HW5)
     * - Enable IE (global interrupt enable)
     */
    MFC0(status, 12, 0);
    status &= ~(ST_BEV | ST_EXL | ST_ERL);
    status |= ST_IM7 | ST_IE;
    MTC0(status, 12, 0);
}

void c_irq_handler() {
	
	kprintf("IRQ");
	timer_interrupt_handler();
	
	
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
void hey_f(void) {
    kprintf("hey ok\n\r");
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
// https://github.com/rickgaiser/kernelloader/blob/ce0fb430a23660772f51de46e726dc34ccb915a6/kernel/interrupts.S#L65
// TODO(michal4132): fix
// interrupt tester
void int_f(void) {
    uint32_t addr = 0x81000000;
    kprintf("%u\n\r", addr);
    kprintf("%a\n\r", addr);
    c0_clrbits(CP0_STATUS, ST0_BEV | ST0_EXL | ST0_ERL);
    c0_setbits(CP0_STATUS, 0xff00); // enable all interrupts
    c0_setbits(CP0_STATUS, ST0_IE);
    kprintf("OK\n\r");

}
static inline uint32_t mfc0_status(void) {
    uint32_t value;
    asm volatile("mfc0 %0, $12" : "=r"(value));
    return value;
}

static inline void mtc0_status(uint32_t value) {
    asm volatile("mtc0 %0, $12" :: "r"(value));
}

#define SR_IEC          0x00000001
void IRQ_EnableInterrupt(void) {
    uint32_t status = mfc0_status();
    uint32_t mask = 6;       // Check ECL (bit 2) or EXL (bit 1)
    
    if ((status & mask) == 0) {
        status |= SR_IEC;    // Set IEC bit
        mtc0_status(status);
    }
}
void int2_f(void) {
IRQ_EnableInterrupt();



}

void run_f(void) {
    kprintf("load from ram\n\r");
    exec(0x83000000);
}


#define SET_BIT(arr, idx, val) \
    do { \
        if(val) \
            (arr)[(idx)/8] |=  (1 << ((idx)%8)); \
        else \
            (arr)[(idx)/8] &= ~(1 << ((idx)%8)); \
    } while(0)

#define GET_BIT(arr, idx) (((arr)[(idx)/8] >> ((idx)%8)) & 1)

#define PIN_MOVE 9
#define PIN_ENTER 31

void pins_f() {
	
	if(1) {
		for(int i = 0; i < GPIO_PORT_MAX; i++)
			gpio_direction_input(i);

		uint8_t gpio_state[(GPIO_PORT_MAX + 7)/8] = {0};

		while(1) {
			for(int i = 0; i < GPIO_PORT_MAX; i++) {
				int val = gpio_get_value(i);
				int old = GET_BIT(gpio_state, i);

				if(val != old) {
					kprintf("Pin %d is now %s!\n\r", i, val ? "UP" : "DOWN");
					SET_BIT(gpio_state, i, val);
				}
			}
		}
	}
}
static inline void patch_vector(uint32_t vec_addr, void (*handler)(void)) {
    uint32_t inst = 0x08000000 | (((uint32_t)handler >> 2) & 0x03FFFFFF);
    volatile uint32_t *vec = (volatile uint32_t *)vec_addr;
    vec[0] = inst;   // jump to handler
    vec[1] = 0;      // nop after jump for delay slot
    __asm__ volatile("sync");  // ensure instruction writes are visible
}

void patch() {
	patch_vector(0x80000180,c_irq_handler);
}
void patch2() {
	patch_vector(0xBFC00000,c_irq_handler);
}
Cmd cmd[] = {
    {"cmd", cmd_f},
    {"hey", hey_f},
    {"int", int_f},
    {"int2", int2_f},
    {"pins", pins_f},
    {"timer", setup_timer},
    {"patch", patch},
    {"patch2", patch2},
};
int cur = 0;

void OS_DisableInterrupt(void);
void OS_EnableInterrupt(void);





void menuProgram() {
    kprintf("Menu!\n\r");
	
	OS_EnableInterrupt();

    gpio_direction_input(PIN_MOVE);
    gpio_direction_input(PIN_ENTER);

    int last_move = gpio_get_value(PIN_MOVE);
    int last_enter = gpio_get_value(PIN_ENTER);

    while (1) {
        int state_move = gpio_get_value(PIN_MOVE);
        int state_enter = gpio_get_value(PIN_ENTER);

        // Detect rising edge for MOVE button
        if (state_move && !last_move) {
            cur++;
            if (cur >= sizeof(cmd)/sizeof(cmd[0]))
				cur = 0;
			
			kprintf("%d\n\r", tick_count);
			kprintf("-> %s\n\r", cmd[cur].name);
        }

        // Detect rising edge for ENTER button
        if (state_enter && !last_enter) {
			kprintf("Running %s\n\r", cmd[cur].name);
            cmd[cur].f();
        }


        last_move = state_move;
        last_enter = state_enter;

    }
}

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

	UINT32 id = sys_ic_get_chip_id_raw();
    kprintf("chip id raw: %x\n\r", id);
	
	menuProgram();

    
    /* Initialize timer interrupt */
    
    /* Main loop */
    while (1) {
    }
    
    // command loop
    while (1) {
        readline(line, 100);
		kprintf("Got %s\n\r", line);
        for (uint8_t i = 0; i < sizeof(cmd)/sizeof(cmd[0]); i++) {
            if (strncmp(line, cmd[i].name, 100) == 0) {
                cmd[i].f();
                break;
            }
        }
    }

    return 0;
}
