#include "uart.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <print.h>

// number of uart ports
#define SCI_16550_NUM 2

// uart address
#define UART_REGS {{0xb8018300, 24, 0}, {0xb8001600, 22, 0}}

#define SUCCESS 0
#define RET_FAILURE 1
#define ERR_FAILUE 2

#define SCI_PARITY_NONE 0x0000
#define SCI_PARITY_EVEN 0x0001
#define SCI_PARITY_ODD 0x0002

#define SCI_16550_URBR 0
#define SCI_16550_UTBR 0
#define SCI_16550_UIER 1
#define SCI_16550_UIIR 2
#define SCI_16550_UFCR 2
#define SCI_16550_UDLL 0
#define SCI_16550_UDLM 1
#define SCI_16550_ULCR 3
#define SCI_16550_UMCR 4
#define SCI_16550_ULSR 5
#define SCI_16550_UMSR 6
#define SCI_16550_USCR 7
#define SCI_16550_DEVC 8
#define SCI_16550_RCVP 9

#define SCI_16550UART_RX_BUF_SIZE 256

static struct sci_uart_st {
    uint8_t rx_buf[SCI_16550UART_RX_BUF_SIZE];
    uint32_t rx_buf_head;
    uint32_t rx_buf_tail;
    uint8_t loopback_flag;
    uint32_t rd_mutex_id;
    uint32_t wt_mutex_id;
    uint32_t timeout;
} *sci_16550 = NULL;

static struct {
    uint32_t reg_base;
    int irq;
    uint32_t strap_ctrl;
} sci_16550_reg[SCI_16550_NUM] = UART_REGS;

#define SCI_READ8(id, reg)                                                     \
    (*((volatile uint8_t *)(sci_16550_reg[id].reg_base + reg)))
#define SCI_WRITE8(id, reg, data)                                              \
    (*((volatile uint8_t *)(sci_16550_reg[id].reg_base + reg)) = (data))

int32_t uart_attach(uint32_t dev_num) {
    int i;

    if (dev_num > 0) {
        dev_num = (dev_num > SCI_16550_NUM) ? SCI_16550_NUM : dev_num;
        sci_16550 =
            (struct sci_uart_st *) malloc (sizeof(struct sci_uart_st) * dev_num);
        if (sci_16550 == NULL)
            return -1;
        memset(sci_16550, 0, sizeof(struct sci_uart_st) * dev_num);

        for (i = 0; i < dev_num; i++) {
            sci_16550[i].rx_buf_head = sci_16550[i].rx_buf_tail = 0;
            sci_16550[i].loopback_flag = 0;
            if (sci_16550_reg[i].strap_ctrl != 0)
                *(uint32_t *)0xb8000074 = sci_16550_reg[i].strap_ctrl;
        }
    }
    return 0;
}

void uart_set_mode(uint32_t id, uint32_t bps, int parity) {
    unsigned int div;

    // disable interrupts
    SCI_WRITE8(id, SCI_16550_UIER, 0);

    sci_16550[id].timeout = 20000;  // 10000000 / bps; Tout = 10*10^6/Rband *//*while write

    // Disable all interrupt
    SCI_WRITE8(id, SCI_16550_UIER, 0);

    div = 115200 / bps;
    // Enable setup baud rate
    SCI_WRITE8(id, SCI_16550_ULCR, 0x9b);
    SCI_WRITE8(id, SCI_16550_UDLL, (div & 0xff));
    SCI_WRITE8(id, SCI_16550_UDLM, ((div >> 8) & 0xff));

    div = (((parity >> 6) & 0x04) | ((~(parity >> 4)) & 0x03));
    switch (parity & 0x03) {
    case SCI_PARITY_EVEN:
        SCI_WRITE8(id, SCI_16550_ULCR, 0x18 | div);  // even parity
        break;
    case SCI_PARITY_ODD:
        SCI_WRITE8(id, SCI_16550_ULCR, 0x08 | div);  // odd parity
        break;
    default:
        SCI_WRITE8(id, SCI_16550_ULCR, 0x00 | div);  // none parity
        break;
    };

    SCI_WRITE8(id, SCI_16550_UFCR, 0x47);  // Reset FIFO

    SCI_WRITE8(id, SCI_16550_ULSR, 0x00);  // Reset line status
    SCI_WRITE8(id, SCI_16550_UMCR, 0x03);  // Set modem control

    // Enable receiver interrupt
    SCI_WRITE8(id, SCI_16550_UIER, 0x05);  // Enable RX & timeout interrupt
}

// TODO(michal4132): implement id
uint8_t uart_read_char(uint32_t id) {
    uint8_t istatus;
    while (1) {
        while (((istatus = (SCI_READ8(0, SCI_16550_UIIR) & 0x0f)) & 1) == 0) {
            switch (istatus) {
            case 0x06:  // LSR error: OE, PE, FE, or BI
                if (SCI_READ8(0, SCI_16550_ULSR) & 0x9e) {
                    kprintf("sci_16550uart_interrupt: lstatus error!\n");
                }
                // We continue receive data at this condition
            case 0x0c:  // Character Timer-outIndication
            case 0x04:  // Received Data Available
                while (SCI_READ8(0, SCI_16550_ULSR) & 1) {
                    return SCI_READ8(0, SCI_16550_URBR);
                }
                break;
            case 0x02:  // TransmitterHoldingRegister Empty
            case 0x00:  // Modem Status
            default:
                break;
            }
        }
    }
    kprintf("read char error\n");
}

void uart_write_char(uint32_t id, uint8_t ch) {
    int i;
    int retry = 3;

    while (retry) {
        // Send character
        SCI_WRITE8(id, SCI_16550_UTBR, ch);

        // wait for transmission finished
        i = sci_16550[id].timeout;
        while (--i) {
            if (SCI_READ8(id, SCI_16550_ULSR) & 0x20) {
                break;
            }
        }
        if (0 != i) {
            break;
        }

        // Timeout, reset XMIT FIFO
        SCI_WRITE8(id, SCI_16550_UFCR, SCI_READ8(id, SCI_16550_UFCR) | 0x04);
        retry--;
    }
    return;
}
