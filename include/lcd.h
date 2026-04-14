#ifndef LCD_H
#define LCD_H

#include <stdint.h>

/* PL111 CLCD Controller Base Address */
#define CLCD_BASE           0x4001F000

/* PL111 Registers */
#define CLCD_TIM0           (*(volatile uint32_t *)(CLCD_BASE + 0x00))
#define CLCD_TIM1           (*(volatile uint32_t *)(CLCD_BASE + 0x04))
#define CLCD_TIM2           (*(volatile uint32_t *)(CLCD_BASE + 0x08))
#define CLCD_TIM3           (*(volatile uint32_t *)(CLCD_BASE + 0x0C))
#define CLCD_UPBASE         (*(volatile uint32_t *)(CLCD_BASE + 0x10))
#define CLCD_LPBASE         (*(volatile uint32_t *)(CLCD_BASE + 0x14))
#define CLCD_CONTROL        (*(volatile uint32_t *)(CLCD_BASE + 0x1C))
#define CLCD_IMSC           (*(volatile uint32_t *)(CLCD_BASE + 0x20))
#define CLCD_RIS            (*(volatile uint32_t *)(CLCD_BASE + 0x24))
#define CLCD_MIS            (*(volatile uint32_t *)(CLCD_BASE + 0x28))
#define CLCD_ICR            (*(volatile uint32_t *)(CLCD_BASE + 0x2C))
#define CLCD_UPCURS         (*(volatile uint32_t *)(CLCD_BASE + 0x30))
#define CLCD_LPCURS         (*(volatile uint32_t *)(CLCD_BASE + 0x34))

/* LCD Control Bits */
#define CLCD_CTRL_EN        (1 << 0)
#define CLCD_CTRL_BPP16     (4 << 1)  /* 16-bit 5:6:5 */
#define CLCD_CTRL_BPP16_555 (3 << 1)
#define CLCD_CTRL_BGR       (1 << 8)
#define CLCD_CTRL_PWR       (1 << 11)

/* Screen Dimensions */
#define LCD_WIDTH           320
#define LCD_HEIGHT          240

/* Premium Colors (RGB 565) */
#define COLOR_BLACK         0x0000
#define COLOR_WHITE         0xFFFF
#define COLOR_DARK_BG       0x2104   /* Deep Charcoal */
#define COLOR_ACCENT_BLUE   0x4D3F   /* Modern Blue */
#define COLOR_ACCENT_PURPLE 0x919B   /* Vibrant Purple */
#define COLOR_SUCCESS_GREEN 0x2E66   /* Emerald */
#define COLOR_DANGER_RED    0xE124   /* Ruby */
#define COLOR_GOLD          0xCE60   /* Warm Gold */

/* Framebuffer Address: 0x20300000 (Safe spot in 4MB SRAM) */
#define LCD_FB_ADDR         0x20300000
#define LCD_FB              ((volatile uint16_t *)LCD_FB_ADDR)

/* Functions */
void lcd_init(void);
void lcd_clear(uint16_t color);
void lcd_draw_pixel(int x, int y, uint16_t color);
void lcd_draw_rect(int x, int y, int w, int h, uint16_t color);
void lcd_draw_char(int x, int y, char c, uint16_t color, uint16_t bg);
void lcd_draw_string(int x, int y, const char *str, uint16_t color, uint16_t bg);

#endif /* LCD_H */
