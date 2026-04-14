/* startup.c — Fixed for QEMU MPS2-AN385 Cortex-M3 */

#include <stdint.h>

extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;
extern uint32_t _estack;

int main(void);

void Default_Handler(void) { for(;;); }

void HardFault_Handler(void)
{
    /* Print something via UART directly */
    volatile uint32_t *uart_data  = (volatile uint32_t*)0x40004000;
    volatile uint32_t *uart_state = (volatile uint32_t*)0x40004004;
    volatile uint32_t *uart_ctrl  = (volatile uint32_t*)0x40004008;
    volatile uint32_t *uart_baud  = (volatile uint32_t*)0x40004010;
    *uart_baud = 16;
    *uart_ctrl = 0x3;
    const char *msg = "\r\n[HARDFAULT]\r\n";
    while (*msg) {
        while (*uart_state & 1);
        *uart_data = (uint32_t)(*msg++);
    }
    for(;;);
}

void Reset_Handler    (void);
void NMI_Handler      (void) __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void) __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler (void) __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void)__attribute__((weak, alias("Default_Handler")));
void SVC_Handler      (void) __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler   (void) __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler  (void) __attribute__((weak, alias("Default_Handler")));
void UART0_Handler    (void) __attribute__((weak, alias("Default_Handler")));

__attribute__((section(".isr_vector")))
void (* const vector_table[])(void) =
{
    (void(*)(void))(&_estack),
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    MemManage_Handler,
    BusFault_Handler,
    UsageFault_Handler,
    0, 0, 0, 0,
    SVC_Handler,
    0,
    0,
    PendSV_Handler,
    SysTick_Handler,
    UART0_Handler,
};

void Reset_Handler(void)
{
    uint32_t *src, *dst;

    src = &_sidata;
    dst = &_sdata;
    while (dst < &_edata)
        *dst++ = *src++;

    dst = &_sbss;
    while (dst < &_ebss)
        *dst++ = 0;

    main();
    for (;;);
}
