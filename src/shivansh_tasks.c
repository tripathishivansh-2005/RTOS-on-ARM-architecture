/*
 * shivansh_tasks.c — Module 3: Memory Systems & Bus
 *
 * Kya karta hai:
 *   Task_Sensor   — Priority 3, sends data via Queue_A
 *   Task_Process  — Priority 2, receives from Queue_A, sends to Queue_B
 *   Task_Display  — Priority 1, receives from Queue_B, prints result
 *   semaphore_demo() — Binary semaphore: signal + wait pattern
 *
 * Memory map print karta hai on startup — Shivansh ka CSA content
 *
 * CSA Connection: SRAM layout, Queue = circular buffer in heap,
 *                 Semaphore = synchronisation primitive,
 *                 Memory hierarchy in QEMU
 */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <stdio.h>
#include <stdint.h>

/* ── Queue handles ─────────────────────────────────────── */
QueueHandle_t Queue_A = NULL;   /* Sensor → Process  */
QueueHandle_t Queue_B = NULL;   /* Process → Display */

/* ── Semaphore handle ──────────────────────────────────── */
static SemaphoreHandle_t xSemaphore = NULL;
static SemaphoreHandle_t xMutex     = NULL;

/* ── Shared counter (protected by mutex) ──────────────── */
static volatile uint32_t shared_counter = 0;

/* ── Memory map print (called once from main) ──────────── */
void shivansh_memory_map(void)
{
    /* These symbols come from the linker script */
    extern uint32_t _sdata, _edata;
    extern uint32_t _sbss,  _ebss;
    extern uint32_t _estack;

    printf("\n[SHIVANSH] ═══ Memory Map (QEMU MPS2) ═══\n");
    printf("[SHIVANSH] Flash  : 0x00000000 — 0x003FFFFF (4MB)\n");
    printf("[SHIVANSH]   .text starts at 0x00000000 (code)\n");
    printf("[SHIVANSH] SRAM   : 0x20000000 — 0x203FFFFF (4MB)\n");
    printf("[SHIVANSH]   .data : 0x%08lX — 0x%08lX\n",
           (unsigned long)&_sdata, (unsigned long)&_edata);
    printf("[SHIVANSH]   .bss  : 0x%08lX — 0x%08lX\n",
           (unsigned long)&_sbss,  (unsigned long)&_ebss);
    printf("[SHIVANSH]   Stack : grows DOWN from 0x%08lX\n",
           (unsigned long)&_estack);
    printf("[SHIVANSH]   Heap  : 64KB for FreeRTOS (TCBs + stacks)\n");
    printf("[SHIVANSH] Peripheral region: 0x40000000+\n");
    printf("[SHIVANSH]   UART0 : 0x40004000\n");
    printf("[SHIVANSH] ═════════════════════════════\n\n");
}

/* ── Task 1: Sensor Task (Priority 3) ───────────────────
 * Generates sensor data, sends to Queue_A
 * Period: 300ms
 */
void Task_Sensor(void *pvParameters)
{
    (void)pvParameters;
    TickType_t xLastWake = xTaskGetTickCount();
    uint32_t sensor_val = 0;

    for (;;) {
        sensor_val += 5;   /* Simulated ADC reading */

        /* Send to Queue_A (non-blocking — drop if full) */
        BaseType_t sent = xQueueSend(Queue_A, &sensor_val, 0);

        /* Also give semaphore every 3 cycles */
        static uint32_t cycle = 0;
        cycle++;
        if (cycle % 3 == 0) {
            xSemaphoreGive(xSemaphore);
            printf("[SHIVANSH] IPC SEM_GIVEN\n");
        }

        /* Mutex-protected counter update */
        if (xSemaphoreTake(xMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            shared_counter++;
            printf("[SHIVANSH] IPC MUTEX_LOCKED shared_ctr=%lu\n", (unsigned long)shared_counter);
            xSemaphoreGive(xMutex);
            printf("[SHIVANSH] IPC MUTEX_FREE\n");
        }

        UBaseType_t wm = uxTaskGetStackHighWaterMark(NULL);
        printf("[SHIVANSH] SENSOR tick=%-6lu val=%-5lu qA=%u queued=%s stk=%u\n",
               (unsigned long)xTaskGetTickCount(),
               (unsigned long)sensor_val,
               (unsigned)uxQueueMessagesWaiting(Queue_A),
               sent == pdTRUE ? "OK " : "DROP",
               (unsigned)wm);

        vTaskDelayUntil(&xLastWake, pdMS_TO_TICKS(300));
    }
}

/* ── Task 2: Process Task (Priority 2) ──────────────────
 * Receives from Queue_A, computes rolling average, sends to Queue_B
 * Period: blocks on queue
 */
void Task_Process(void *pvParameters)
{
    (void)pvParameters;
    uint32_t raw_val = 0;
    uint32_t buf[5]  = {0};
    uint8_t  idx     = 0;
    uint32_t avg     = 0;

    for (;;) {
        /* Block until data arrives in Queue_A (1 second max) */
        if (xQueueReceive(Queue_A, &raw_val, pdMS_TO_TICKS(1000)) == pdTRUE) {
            /* Rolling average of last 5 samples */
            buf[idx % 5] = raw_val;
            idx++;
            uint32_t sum = 0;
            for (int i = 0; i < 5; i++) sum += buf[i];
            avg = sum / 5;

            /* Forward average to Display */
            xQueueSend(Queue_B, &avg, 0);

            /* Also wait for semaphore sometimes */
            if (xSemaphoreTake(xSemaphore, 0) == pdTRUE) {
                printf("[SHIVANSH] IPC SEM_TAKEN\n");
                printf("[SHIVANSH] PROCESS semaphore received! "
                       "raw=%-5lu avg=%-5lu qA=%u qB=%u\n",
                       (unsigned long)raw_val, (unsigned long)avg,
                       (unsigned)uxQueueMessagesWaiting(Queue_A),
                       (unsigned)uxQueueMessagesWaiting(Queue_B));
            } else {
                UBaseType_t wm = uxTaskGetStackHighWaterMark(NULL);
                printf("[SHIVANSH] PROCESS tick=%-6lu raw=%-5lu avg=%-5lu qA=%u qB=%u stk=%u\n",
                       (unsigned long)xTaskGetTickCount(),
                       (unsigned long)raw_val, (unsigned long)avg,
                       (unsigned)uxQueueMessagesWaiting(Queue_A),
                       (unsigned)uxQueueMessagesWaiting(Queue_B),
                       (unsigned)wm);
            }
        }
    }
}

/* ── Task 3: Display Task (Priority 1) ──────────────────
 * Receives processed average from Queue_B, "displays" it
 * Shows memory layout info periodically
 */
void Task_Display(void *pvParameters)
{
    (void)pvParameters;
    uint32_t display_val = 0;
    static uint32_t run = 0;

    for (;;) {
        if (xQueueReceive(Queue_B, &display_val, pdMS_TO_TICKS(2000)) == pdTRUE) {
            run++;
            UBaseType_t wm = uxTaskGetStackHighWaterMark(NULL);
            printf("[SHIVANSH] DISPLAY tick=%-6lu avg=%-5lu qB=%u stk=%u\n",
                   (unsigned long)xTaskGetTickCount(),
                   (unsigned long)display_val,
                   (unsigned)uxQueueMessagesWaiting(Queue_B),
                   (unsigned)wm);

            /* Periodically print queue info */
            if (run % 5 == 0) {
                printf("[SHIVANSH]  >> Queue info:\n");
                printf("[SHIVANSH]     Queue_A spaces: %u\n",
                       (unsigned)uxQueueSpacesAvailable(Queue_A));
                printf("[SHIVANSH]     Queue_B spaces: %u\n",
                       (unsigned)uxQueueSpacesAvailable(Queue_B));
                printf("[SHIVANSH]  >> Queue concept: circular buffer in SRAM heap\n");
                printf("[SHIVANSH]     xQueueCreate(5, sizeof(uint32_t)) = 20 bytes\n");
            }
        }
    }
}

/* ── Init function (called from main before scheduler) ── */
void shivansh_queues_init(void)
{
    Queue_A    = xQueueCreate(5, sizeof(uint32_t));
    Queue_B    = xQueueCreate(5, sizeof(uint32_t));
    xSemaphore = xSemaphoreCreateBinary();
    xMutex     = xSemaphoreCreateMutex();

    if (!Queue_A || !Queue_B || !xSemaphore || !xMutex) {
        printf("[SHIVANSH] ERROR: Queue/Semaphore creation failed!\n");
        for (;;);
    }
    printf("[SHIVANSH] Queues + Semaphore + Mutex created OK\n");
}
