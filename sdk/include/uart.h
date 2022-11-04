#ifndef _UART_H_
#define _UART_H_

#include <stdint.h>

// typedef         unsigned char uint8;
// typedef         unsigned int  uint32;

#define         UART16550_BAUD_2400             2400
#define         UART16550_BAUD_4800             4800
#define         UART16550_BAUD_9600             9600
#define         UART16550_BAUD_19200            19200
#define         UART16550_BAUD_38400            38400
#define         UART16550_BAUD_57600            57600
#define         UART16550_BAUD_115200           115200

#define         UART16550_PARITY_NONE           0
#define         UART16550_PARITY_ODD            0x08
#define         UART16550_PARITY_EVEN           0x18
#define         UART16550_PARITY_MARK           0x28
#define         UART16550_PARITY_SPACE          0x38

#define         UART16550_DATA_5BIT             0x0
#define         UART16550_DATA_6BIT             0x1
#define         UART16550_DATA_7BIT             0x2
#define         UART16550_DATA_8BIT             0x3

#define         UART16550_STOP_1BIT             0x0
#define         UART16550_STOP_2BIT             0x4

// void Uart16550Init(uint32_t baud, uint8_t data, uint8_t parity, uint8_t stop);
int32_t uart_attach(uint32_t dev_num);
void uart_set_mode(uint32_t id, uint32_t bps, int parity);
uint8_t uart_read_char(uint32_t id);
void uart_write_char(uint32_t id, uint8_t ch);

#endif // _UART_H_
