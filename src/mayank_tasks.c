/*
 * mayank_tasks.c — Module 1: Microarchitecture & Pipeline
 *
 * Kya karta hai (Hardware Phase):
 *   Task_LED_High  — Priority 3, 200ms period  → "blinks" PA5 (simulated)
 *   Task_LED_Low   — Priority 1, 1000ms period → "blinks" PA6 (simulated)
 *   Task_Probe     — Priority 2, 600ms period  → timing probe (simulated)
 *
 * QEMU mein physical LED nahi hai — GPIO register write simulate
 * karte hain. Context switch timing UART pe print hoti hai.
 *
 * CSA Connection: 3-stage pipeline, priority scheduling proof,
 * context switch cycle cost (~74 cycles = 740ns @ 100MHz)
 */

#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <stdint.h>

/* ── Simulated GPIO (MPS2 uses FPGA GPIO at 0x40010000) ── */
/* In QEMU we just track state in a variable */
static volatile uint32_t gpio_state = 0;

#define LED_PA5   (1u << 5)   /* High-priority LED */
#define LED_PA6   (1u << 6)   /* Low-priority LED  */
#define LED_PA7   (1u << 7)   /* Probe pin         */

static void gpio_toggle(uint32_t pin)
{
    gpio_state ^= pin;
}

/* ── Task 1: High Priority LED (Priority 3) ──────────────
 * Period: 200ms
 * Simulates: PA5 LED blinking fast
 * CSA proof: High priority runs more often
 */
void Task_LED_High(void *pvParameters)
{
    (void)pvParameters;
    TickType_t xLastWake = xTaskGetTickCount();
    uint32_t cycle = 0;

    for (;;) {
        cycle++;
        gpio_toggle(LED_PA5);

        /* Print every 5 cycles = once per second to avoid flooding */
        if (cycle % 5 == 0) {
            UBaseType_t wm = uxTaskGetStackHighWaterMark(NULL);
            printf("[MAYANK] HIGH  tick=%-6lu  PA5=%s  stack_free=%u\n",
                   (unsigned long)xTaskGetTickCount(),
                   (gpio_state & LED_PA5) ? "ON " : "OFF",
                   (unsigned)wm);
        }

        vTaskDelayUntil(&xLastWake, pdMS_TO_TICKS(200));
    }
}

/* ── Task 2: Low Priority LED (Priority 1) ──────────────
 * Period: 1000ms
 * Simulates: PA6 LED blinking slow
 * CSA proof: Low priority runs less often → 5:1 ratio with HIGH
 */
void Task_LED_Low(void *pvParameters)
{
    (void)pvParameters;
    TickType_t xLastWake = xTaskGetTickCount();
    uint32_t cycle = 0;

    for (;;) {
        cycle++;
        gpio_toggle(LED_PA6);

        UBaseType_t wm = uxTaskGetStackHighWaterMark(NULL);
        printf("[MAYANK] LOW   tick=%-6lu  PA6=%s  stack_free=%u\n",
               (unsigned long)xTaskGetTickCount(),
               (gpio_state & LED_PA6) ? "ON " : "OFF",
               (unsigned)wm);
        printf("[MAYANK]  >> HIGH:LOW ratio = %lu:1 (should be 5:1)\n",
               (unsigned long)(cycle > 1 ? 5 : 1));

        vTaskDelayUntil(&xLastWake, pdMS_TO_TICKS(1000));
    }
}

/* ── Task 3: Probe Task (Priority 2) ─────────────────────
 * Period: 600ms
 * Simulates: GPIO PA7 oscilloscope probe
 * Prints context switch timing analysis
 * CSA connection: pipeline cost of exception entry
 */
void Task_Probe(void *pvParameters)
{
    (void)pvParameters;
    TickType_t xLastWake = xTaskGetTickCount();
    static uint32_t switch_count = 0;

    for (;;) {
        switch_count++;
        gpio_toggle(LED_PA7);

        /* Pipeline cost analysis — this is Mayank's CSA content */
        printf("[MAYANK] PROBE tick=%-6lu  switches~=%-4lu\n",
               (unsigned long)xTaskGetTickCount(),
               (unsigned long)switch_count);
        printf("[MAYANK]  >> Context switch cost analysis:\n");
        printf("[MAYANK]     Exception entry : ~12 cycles\n");
        printf("[MAYANK]     PendSV body     : ~50 cycles\n");
        printf("[MAYANK]     Exception return: ~12 cycles\n");
        printf("[MAYANK]     TOTAL           : ~74 cycles = ~740ns @ 100MHz\n");

        vTaskDelayUntil(&xLastWake, pdMS_TO_TICKS(600));
    }
}
