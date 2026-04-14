/*
 * main.c — ARM Cortex-M RTOS Project on QEMU
 * Team: Mayank, Deepanshu, Shivansh, Yash
 */

#include "FreeRTOS.h"
#include "task.h"
#include "uart.h"
#include "lcd.h"
#include <stdio.h>

/* Forward declarations */
void Task_LED_High  (void *pvParameters);
void Task_LED_Low   (void *pvParameters);
void Task_Probe     (void *pvParameters);

void Task_ISA_High  (void *pvParameters);
void Task_ISA_Medium(void *pvParameters);
void Task_ISA_Low   (void *pvParameters);
void deepanshu_startup_trace(void);

void Task_Sensor (void *pvParameters);
void Task_Process(void *pvParameters);
void Task_Display(void *pvParameters);
void shivansh_queues_init(void);
void shivansh_memory_map(void);

void Task_Heartbeat(void *pvParameters);
void Task_OS_Info  (void *pvParameters);
void yash_exception_demo(void);
void yash_systick_info  (void);
void Task_Visual       (void *pvParameters);

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    printf("\r\n!!! STACK OVERFLOW: %s !!!\r\n", pcTaskName);
    for (;;);
}

void vApplicationMallocFailedHook(void)
{
    printf("\r\n!!! MALLOC FAILED !!!\r\n");
    for (;;);
}

/* Idle hook required when configUSE_IDLE_HOOK=0 but just in case */
void vApplicationIdleHook(void) { }

#define STACK_SMALL   (256)
#define STACK_MEDIUM  (384)
#define STACK_LARGE   (512)

int main(void)
{
    uart_init();
    lcd_init();

    printf("\r\n==============================================\r\n");
    printf("  ARM Cortex-M RTOS on QEMU MPS2-AN385\r\n");
    printf("  FreeRTOS + 11 Tasks - Team Project Demo\r\n");
    printf("  Mayank | Deepanshu | Shivansh | Yash\r\n");
    printf("==============================================\r\n\r\n");

    deepanshu_startup_trace();
    shivansh_memory_map();
    yash_exception_demo();
    yash_systick_info();

    shivansh_queues_init();

    printf("[MAIN] Creating tasks...\r\n");

    xTaskCreate(Task_LED_High,   "LED_High",  STACK_SMALL,  NULL, 3, NULL);
    xTaskCreate(Task_LED_Low,    "LED_Low",   STACK_SMALL,  NULL, 1, NULL);
    xTaskCreate(Task_Probe,      "Probe",     STACK_MEDIUM, NULL, 2, NULL);

    xTaskCreate(Task_ISA_High,   "ISA_High",  STACK_MEDIUM, NULL, 3, NULL);
    xTaskCreate(Task_ISA_Medium, "ISA_Med",   STACK_MEDIUM, NULL, 2, NULL);
    xTaskCreate(Task_ISA_Low,    "ISA_Low",   STACK_SMALL,  NULL, 1, NULL);

    xTaskCreate(Task_Sensor,  "Sensor",  STACK_MEDIUM, NULL, 3, NULL);
    xTaskCreate(Task_Process, "Process", STACK_LARGE,  NULL, 2, NULL);
    xTaskCreate(Task_Display, "Display", STACK_LARGE,  NULL, 1, NULL);

    xTaskCreate(Task_Heartbeat, "Heartbeat", STACK_SMALL,  NULL, 1, NULL);
    xTaskCreate(Task_OS_Info,   "OS_Info",   STACK_LARGE,  NULL, 2, NULL);
    xTaskCreate(Task_Visual,    "Visual",    STACK_LARGE,  NULL, 1, NULL);

    printf("[MAIN] All 11 tasks created. Starting scheduler...\r\n\r\n");

    vTaskStartScheduler();

    printf("[MAIN] ERROR: Scheduler returned!\r\n");
    for (;;);
    return 0;
}
