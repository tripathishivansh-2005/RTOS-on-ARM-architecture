/*
 * syscalls.c — printf redirect to MPS2 UART0
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include "uart.h"

/* Called by newlib for write() / printf() */
int _write(int file, char *ptr, int len)
{
    (void)file;
    for (int i = 0; i < len; i++) {
        if (ptr[i] == '\n')
            uart_putc('\r');
        uart_putc(ptr[i]);
    }
    __asm volatile("DSB" ::: "memory");
    __asm volatile("ISB" ::: "memory");
    return len;
}

/* Stub implementations required by newlib */
int _read (int file, char *ptr, int len)  { (void)file;(void)ptr;(void)len; return 0; }
int _close(int file)                       { (void)file; return -1; }
int _fstat(int file, struct stat *st)     { (void)file; st->st_mode = S_IFCHR; return 0; }
int _isatty(int file)                      { (void)file; return 1; }
int _lseek(int file, int ptr, int dir)    { (void)file;(void)ptr;(void)dir; return 0; }
void *_sbrk(int incr)
{
    extern uint8_t _end;
    static uint8_t *heap_end = 0;
    uint8_t *prev_heap_end;
    if (heap_end == 0) heap_end = &_end;
    prev_heap_end = heap_end;
    heap_end += incr;
    return (void*)prev_heap_end;
}
