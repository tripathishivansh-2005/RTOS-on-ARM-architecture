/*
 * yash_tasks.c — Module 4: OS Integration & System Control
 *
 * Kya karta hai:
 *   Task_Heartbeat — Priority 1, 500ms → system alive proof
 *   Task_OS_Info   — Priority 2, 2000ms → FreeRTOS internals print
 *   yash_exception_demo() — Exception model, vector table, EXC_RETURN
 *   yash_systick_info()   — SysTick RELOAD calculation
 *
 * CSA Connection: NVIC, Vector Table, Exception Entry Frame,
 *                 SysTick (RELOAD=24999 @ 25MHz/1000Hz),
 *                 PendSV, EXC_RETURN=0xFFFFFFFD, MSP/PSP
 */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>
#include <stdint.h>

/* MPS2 SysTick registers (same as Cortex-M standard) */
#define SYST_RVR   (*((volatile uint32_t*)0xE000E014))  /* Reload value */
#define SYST_CVR   (*((volatile uint32_t*)0xE000E018))  /* Current value */
#define SYST_CSR   (*((volatile uint32_t*)0xE000E010))  /* Control */

/* NVIC ISER0 — interrupt enable set register */
#define NVIC_ISER0 (*((volatile uint32_t*)0xE000E100))

/* SCB ICSR — for pending PendSV */
#define SCB_ICSR   (*((volatile uint32_t*)0xE000ED04))

/* ── Exception demo (called once from main) ─────────────
 * Prints vector table structure, exception frame,
 * EXC_RETURN explanation
 */
void yash_exception_demo(void)
{
    printf("\n[YASH] ═══ Exception Model Demo ════════════════\n");

    /* Vector table */
    printf("[YASH] Vector table at 0x00000000 (Flash start):\n");
printf("[YASH]   Entry[0]  MSP initial = 0x20004000\n");
    printf("[YASH]   Entry[1]  Reset_Handler = (see vector table)\n");
    printf("[YASH]   Entry[14] PendSV_Handler\n");
    printf("[YASH]   Entry[15] SysTick_Handler\n");

    /* Exception entry frame */
    printf("\n[YASH] Exception Entry — 8 registers auto-saved:\n");
    printf("[YASH]   (High addr) xPSR  ← pushed first\n");
    printf("[YASH]               PC    ← return address\n");
    printf("[YASH]               LR    ← EXC_RETURN value!\n");
    printf("[YASH]               R12\n");
    printf("[YASH]               R3\n");
    printf("[YASH]               R2\n");
    printf("[YASH]               R1\n");
    printf("[YASH]   (Low addr)  R0    ← SP points here\n");
    printf("[YASH]   Total = 8 × 4 = 32 bytes per frame\n");

    /* EXC_RETURN */
    printf("\n[YASH] EXC_RETURN (magic LR value in ISR):\n");
    printf("[YASH]   0xFFFFFFF1 = Return to Handler, use MSP (nested ISR)\n");
    printf("[YASH]   0xFFFFFFF9 = Return to Thread,  use MSP (before RTOS)\n");
    printf("[YASH]   0xFFFFFFFD = Return to Thread,  use PSP ← FreeRTOS uses THIS!\n");

    /* MSP vs PSP */
    printf("\n[YASH] Dual Stack Pointers:\n");
    printf("[YASH]   MSP = Main Stack  → Kernel + ISR handlers\n");
    printf("[YASH]   PSP = Process Stack → Each FreeRTOS task\n");
    printf("[YASH]   Context switch = PSP switch (PendSV_Handler does this)\n");

    printf("[YASH] ═════════════════════════════════════════\n\n");
}

/* ── SysTick info (called once from main) ───────────────
 * Shows RELOAD calculation — Yash's CSA slide 5
 */
void yash_systick_info(void)
{
    uint32_t cpu_hz = configCPU_CLOCK_HZ;
    uint32_t tick_hz = configTICK_RATE_HZ;
    uint32_t reload = (cpu_hz / tick_hz) - 1;

    printf("[YASH] === SysTick Configuration ===\n");
    printf("[YASH]   CPU clock       = %lu Hz (25 MHz)\n", (unsigned long)cpu_hz);
    printf("[YASH]   Tick rate       = %lu Hz (1 tick = 1 ms)\n", (unsigned long)tick_hz);
    printf("[YASH]   RELOAD formula  = (CPU_Hz / Tick_Hz) - 1\n");
    printf("[YASH]   RELOAD actual   = %lu\n", (unsigned long)reload);
    printf("[YASH]   SysTick period  = %lu cycles = 1ms\n", (unsigned long)(reload + 1));
    printf("[YASH] ===========================\n\n");
}

/* ── Task 1: Heartbeat (Priority 1) ─────────────────────
 * Period: 500ms — "I am alive" signal
 * If this stops printing → system is frozen
 */
void Task_Heartbeat(void *pvParameters)
{
    (void)pvParameters;
    TickType_t xLastWake = xTaskGetTickCount();
    uint32_t beat = 0;

    for (;;) {
        beat++;
        UBaseType_t wm = uxTaskGetStackHighWaterMark(NULL);
        printf("[YASH] HB     tick=%-6lu  beat=%-4lu  stk=%u  ♥\n",
               (unsigned long)xTaskGetTickCount(),
               (unsigned long)beat,
               (unsigned)wm);
        vTaskDelayUntil(&xLastWake, pdMS_TO_TICKS(500));
    }
}

/* ── Task 2: OS Info (Priority 2) ───────────────────────
 * Period: 2000ms
 * Prints live FreeRTOS scheduler information
 * Yash's OS integration content
 */
void Task_OS_Info(void *pvParameters)
{
    (void)pvParameters;
    TickType_t xLastWake = xTaskGetTickCount();
    static uint32_t run = 0;

    for (;;) {
        run++;
        printf("\n[YASH] ── OS Info (run %lu) ──\n", (unsigned long)run);

        /* FreeRTOS tick info */
        TickType_t ticks = xTaskGetTickCount();
        printf("[YASH]   xTickCount      = %-6lu (%lu ms elapsed)\n",
               (unsigned long)ticks, (unsigned long)ticks);

        /* Task count */
        printf("[YASH]   Tasks running   = %u\n",
               (unsigned)uxTaskGetNumberOfTasks());

        /* SysTick current value */
        printf("[YASH]   SYST_CVR now    = %lu (counting down)\n",
               (unsigned long)SYST_CVR);

        /* Context switch explanation */
        printf("[YASH]  >> SysTick fires every 1ms →\n");
        printf("[YASH]     xTaskIncrementTick() →\n");
        printf("[YASH]     if switch needed: SCB_ICSR bit28=1 →\n");
        printf("[YASH]     PendSV fires (priority 15, last) →\n");
        printf("[YASH]     save R4-R11, vTaskSwitchContext(), restore R4-R11\n");
        printf("[YASH]     Total cost: ~74 cycles = ~740ns @ 100MHz\n");

        /* Scheduler state */
        printf("[YASH]   Scheduler state = %s\n",
               xTaskGetSchedulerState() == taskSCHEDULER_RUNNING ?
               "RUNNING" : "NOT RUNNING");

        vTaskDelayUntil(&xLastWake, pdMS_TO_TICKS(2000));
    }
}
