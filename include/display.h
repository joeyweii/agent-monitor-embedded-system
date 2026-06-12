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
void display_flush();

#endif // DISPLAY_H
