#include "uart.h"
#include <errno.h>
#include <stdint.h>
#include <sys/stat.h>

extern int _heap_start;

static caddr_t *heap = NULL;

caddr_t sbrk(int incr) {
    caddr_t *prev_heap;

    if (heap == NULL) {
        heap = (caddr_t *)&_heap_start;
    }

    prev_heap = heap;
    heap += incr;

    return (caddr_t)prev_heap;
}

int close(int file) { return -1; }

int fstat(int file, struct stat *st) {
    //  errno = -ENOSYS;
    st->st_mode = S_IFCHR;

    return 0;
}

int isatty(int file) { return 1; }

int lseek(int file, int ptr, int dir) { return 0; }

void _exit(int status) {
    while (1) {
    }
}

void kill(int pid, int sig) {}

int getpid(void) { return -1; }

int write(int file, char *ptr, int len) {
    int written = 0;

    if ((file != 1) && (file != 2) && (file != 3)) {
        return -1;
    }

    for (; len != 0; --len) {
        uart_write_char(0, *ptr++);
        ++written;
    }
    return written;
}

int read(int file, char *ptr, int len) {
    int read = 0;

    if (file != 0) {
        return -1;
    }

    //  for (; len > 0; --len) {
    //    usart_serial_getchar(&stdio_uart_module, (uint8_t *)ptr++);
    //    read++;
    //  }
    return read;
}

int _open(const char *name, int flags, int mode) {
    return 0;
}

void _close(int fd) {}
