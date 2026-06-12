#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

// Display Dimensions (ST7735 1.8")
#define LCD_WIDTH  128
#define LCD_HEIGHT 160

// Basic colors (RGB565 format: RRRRRGGGGGGBBBBB)
#define COLOR_BLACK   0x0000
#define COLOR_RED     0xF800
#define COLOR_GREEN   0x07E0
#define COLOR_BLUE    0x001F
#define COLOR_WHITE   0xFFFF
#define COLOR_YELLOW  0xFFE0
#define COLOR_MAGENTA 0xF81F

// Pin Definitions
#define PIN_SCK   18
#define PIN_MOSI  19
#define PIN_CS    17
#define PIN_DC    20
#define PIN_RESET 21
#define PIN_BL    22

// SPI Instance
#define SPI_PORT spi0

// API Functions
void display_init();
void display_clear(uint16_t color);
void display_draw_pixel(int x, int y, uint16_t color);
void display_draw_rect(int x, int y, int w, int h, uint16_t color);

// DMA & Double Buffering API
void display_flush_async();
bool display_is_busy();
void display_wait_ready();

// Text Rendering API
void display_draw_char(int x, int y, char c, uint16_t color, uint16_t bg, uint8_t size);
void display_draw_string(int x, int y, const char* str, uint16_t color, uint16_t bg, uint8_t size);

#endif // DISPLAY_H
