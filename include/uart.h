/*
 * uart.h — MPS2-AN385 UART0 Driver (fixed for QEMU)
 */

#ifndef UART_H
#define UART_H

#include <stdint.h>

typedef struct {
    volatile uint32_t DATA;
    volatile uint32_t STATE;
    volatile uint32_t CTRL;
    volatile uint32_t INT;
    volatile uint32_t BAUDDIV;
} MPS2_UART_TypeDef;

#define MPS2_UART0  ((MPS2_UART_TypeDef*) 0x40004000UL)

#define UART_STATE_TX_FULL   (1u << 0)
#define UART_CTRL_TX_EN      (1u << 0)
#define UART_CTRL_RX_EN      (1u << 1)

static inline void uart_init(void)
{
    MPS2_UART0->BAUDDIV = 16;
    MPS2_UART0->CTRL    = UART_CTRL_TX_EN | UART_CTRL_RX_EN;
}

static inline void uart_putc(char c)
{
    while (MPS2_UART0->STATE & UART_STATE_TX_FULL);
    MPS2_UART0->DATA = (uint32_t)c;
    while (MPS2_UART0->STATE & UART_STATE_TX_FULL);
}

static inline void uart_puts(const char *s)
{
    while (*s) uart_putc(*s++);
}

#endif /* UART_H */
