/*
 * deepanshu_tasks.c — Module 2: ISA & Registers
 *
 * Kya karta hai:
 *   Task_ISA_High   — Priority 3, 200ms → prints every run
 *   Task_ISA_Medium — Priority 2, 500ms
 *   Task_ISA_Low    — Priority 1, 1000ms
 *   asm_add()       — Thumb-2 assembly function (in asm_add.s)
 *
 * Startup trace bhi included — Reset_Handler steps print hote hain
 * (ek baar main() se call karke).
 *
 * CSA Connection: Register file (R0-R15), Thumb-2 ISA, MSP/PSP,
 *                 AAPCS calling convention, startup sequence
 */

#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <stdint.h>

/* ── Assembly function declaration ───────────────────────
 * Defined in asm_add.s
 * AAPCS: R0=a, R1=b, returns R0=a+b
 */
extern uint32_t asm_add(uint32_t a, uint32_t b);
extern uint32_t asm_mul(uint32_t a, uint32_t b);

/* ── Startup trace (called once from main) ──────────────
 * Simulates what startup.c does — Deepanshu's CSA content
 */
void deepanshu_startup_trace(void)
{
    printf("\n[DEEPANSHU] ═══ Startup Sequence Trace ═══\n");
    printf("[DEEPANSHU] Reset_Handler called by hardware (vector table entry 1)\n");
    printf("[DEEPANSHU]   Step 1: MSP set from vector_table[0] = 0x20004000\n");
    printf("[DEEPANSHU]   Step 2: .data copy Flash→SRAM\n");
    printf("[DEEPANSHU]          src=_sidata (Flash), dst=_sdata..._edata (SRAM)\n");
    printf("[DEEPANSHU]   Step 3: .bss zero fill _sbss..._ebss\n");
    printf("[DEEPANSHU]   Step 4: main() called\n");
    printf("[DEEPANSHU] ═══ Boot complete ════════════\n\n");
}

/* ── Task 1: ISA High Priority (Priority 3) ─────────────
 * Period: 200ms
 * Calls asm_add() every run — Thumb-2 ISA proof
 */
void Task_ISA_High(void *pvParameters)
{
    (void)pvParameters;
    TickType_t xLastWake = xTaskGetTickCount();

    for (;;) {
        /* Thumb-2 assembly function call — AAPCS proof */
        uint32_t result_add = asm_add(10, 20);
        uint32_t result_mul = asm_mul(6, 7);

        UBaseType_t wm = uxTaskGetStackHighWaterMark(NULL);

        printf("[DEEPANSHU] HIGH   tick=%-6lu  asm_add(10,20)=%-3lu  "
               "asm_mul(6,7)=%-3lu  stk=%u\n",
               (unsigned long)xTaskGetTickCount(),
               (unsigned long)result_add,
               (unsigned long)result_mul,
               (unsigned)wm);

        vTaskDelayUntil(&xLastWake, pdMS_TO_TICKS(200));
    }
}

/* ── Task 2: ISA Medium Priority (Priority 2) ───────────
 * Period: 500ms
 * Prints register file info — CSA slide content
 */
void Task_ISA_Medium(void *pvParameters)
{
    (void)pvParameters;
    TickType_t xLastWake = xTaskGetTickCount();
    static uint32_t run_count = 0;

    for (;;) {
        run_count++;
        UBaseType_t wm = uxTaskGetStackHighWaterMark(NULL);

        printf("[DEEPANSHU] MED    tick=%-6lu  run=%-4lu  stk=%u\n",
               (unsigned long)xTaskGetTickCount(),
               (unsigned long)run_count,
               (unsigned)wm);

        /* Every 4 runs — print ISA info */
        if (run_count % 4 == 0) {
            printf("[DEEPANSHU]  >> ARM Register File (AAPCS):\n");
            printf("[DEEPANSHU]     R0-R3  : args/return value (caller-saved)\n");
            printf("[DEEPANSHU]     R4-R11 : general purpose (callee-saved)\n");
            printf("[DEEPANSHU]     R12    : scratch (IP)\n");
            printf("[DEEPANSHU]     R13    : SP (MSP/PSP — see Yash!)\n");
            printf("[DEEPANSHU]     R14    : LR — return address\n");
            printf("[DEEPANSHU]     R15    : PC — points to NEXT instruction\n");
        }

        vTaskDelayUntil(&xLastWake, pdMS_TO_TICKS(500));
    }
}

/* ── Task 3: ISA Low Priority (Priority 1) ──────────────
 * Period: 1000ms
 * Prints Thumb-2 ISA summary — CSA slide content
 */
void Task_ISA_Low(void *pvParameters)
{
    (void)pvParameters;
    TickType_t xLastWake = xTaskGetTickCount();
    static uint32_t run_count = 0;

    for (;;) {
        run_count++;
        UBaseType_t wm = uxTaskGetStackHighWaterMark(NULL);

        printf("[DEEPANSHU] LOW    tick=%-6lu  run=%-4lu  stk=%u\n",
               (unsigned long)xTaskGetTickCount(),
               (unsigned long)run_count,
               (unsigned)wm);

        if (run_count % 3 == 0) {
            printf("[DEEPANSHU]  >> Thumb-2 ISA examples run in asm_add.s:\n");
            printf("[DEEPANSHU]     ADD R0, R0, R1  ; R0 = R0 + R1 (32-bit instr)\n");
            printf("[DEEPANSHU]     MUL R0, R0, R1  ; R0 = R0 * R1 (32-bit instr)\n");
            printf("[DEEPANSHU]     BX  LR          ; return (Thumb mode)\n");
        }

        vTaskDelayUntil(&xLastWake, pdMS_TO_TICKS(1000));
    }
}
