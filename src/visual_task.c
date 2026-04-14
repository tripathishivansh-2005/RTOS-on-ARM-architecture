#include "FreeRTOS.h"
#include "task.h"
#include "lcd.h"
#include <stdio.h>

void Task_Visual(void *pvParameters) {
    (void)pvParameters;
    
    lcd_clear(COLOR_DARK_BG);
    
    /* Draw Header */
    lcd_draw_rect(0, 0, LCD_WIDTH, 40, COLOR_ACCENT_BLUE);
    lcd_draw_string(60, 15, "ARM RTOS PREMIUM DASHBOARD", COLOR_WHITE, 0);
    
    /* Draw Side Info */
    lcd_draw_rect(10, 50, 140, 180, 0x3186); /* Darker gray */
    lcd_draw_string(20, 60, "System Info", COLOR_GOLD, 0);
    lcd_draw_string(20, 80, "CPU: 25MHz", COLOR_WHITE, 0);
    lcd_draw_string(20, 100, "Tasks: 12", COLOR_WHITE, 0);
    lcd_draw_string(20, 120, "Mem: 4MB", COLOR_WHITE, 0);
    
    /* Draw Task List Mockup */
    lcd_draw_string(160, 60, "Live Task Status:", COLOR_ACCENT_PURPLE, 0);
    
    const char *task_names[] = {"LED_High", "Sensor", "Process", "Display", "Heartbeat"};
    uint16_t task_colors[] = {COLOR_SUCCESS_GREEN, COLOR_SUCCESS_GREEN, COLOR_GOLD, COLOR_SUCCESS_GREEN, COLOR_SUCCESS_GREEN};

    int pulse = 0;
    
    for (;;) {
        /* Update Task Bars */
        for (int i = 0; i < 5; i++) {
            int y = 80 + (i * 25);
            lcd_draw_string(160, y, task_names[i], COLOR_WHITE, 0x3186);
            lcd_draw_rect(240, y, 60, 10, COLOR_BLACK);
            lcd_draw_rect(240, y, 10 + (pulse % 50), 10, task_colors[i]);
        }
        
        /* Pulse Animation */
        pulse += 2;
        
        /* Heartbeat icon */
        if (pulse % 20 < 10) {
            lcd_draw_string(10, 15, "<3", COLOR_DANGER_RED, COLOR_ACCENT_BLUE);
        } else {
            lcd_draw_string(10, 15, "  ", COLOR_DANGER_RED, COLOR_ACCENT_BLUE);
        }
        
        /* Moving Progress Bar at bottom */
        static int bar_x = 0;
        lcd_draw_rect(0, 235, LCD_WIDTH, 5, COLOR_BLACK);
        lcd_draw_rect(bar_x, 235, 40, 5, COLOR_ACCENT_BLUE);
        bar_x = (bar_x + 4) % (LCD_WIDTH - 40);

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
