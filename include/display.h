/**
 * @file display.h
 * @brief Low-level display driver and graphics primitives for ST7735 LCD.
 */

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
#define COLOR_GRAY    0x8410
#define COLOR_CYAN    0x07FF
#define COLOR_LIGHT_PINK 0xFAAC
#define COLOR_ROSE    0xFDB7

// Pin Definitions
#define PIN_SCK   18
#define PIN_MOSI  19
#define PIN_CS    17
#define PIN_DC    20
#define PIN_RESET 21
#define PIN_BL    22

// SPI Instance
#define SPI_PORT spi0

// Hardware SPI Configuration
#define SPI_BAUDRATE (24 * 1000 * 1000)

// Power Management
#define BACKLIGHT_TIMEOUT_US 10000000 // 10 seconds
#define BACKLIGHT_FULL_BRIGHTNESS 65535

/**
 * @brief Initializes the display hardware, SPI, PWM, and ST7735 controller.
 */
void display_init();

/**
 * @brief Clears the back buffer with a specific color.
 * @param color RGB565 color value.
 */
void display_clear(uint16_t color);

/**
 * @brief Draws a single pixel to the back buffer.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @param color RGB565 color value.
 */
void display_draw_pixel(int x, int y, uint16_t color);

/**
 * @brief Draws a filled rectangle to the back buffer.
 */
void display_draw_rect(int x, int y, int w, int h, uint16_t color);

/**
 * @brief Draws a hollow rectangle (border) to the back buffer.
 */
void display_draw_hollow_rect(int x, int y, int w, int h, uint16_t color);

/**
 * @brief Swaps buffers and starts an asynchronous DMA transfer to the LCD.
 * Non-blocking if a transfer is not already in progress.
 */
void display_flush_async();

/**
 * @brief Sets the backlight brightness level via PWM.
 * @param level 16-bit PWM duty cycle (0-65535).
 */
void display_set_backlight(uint16_t level);

void display_init_backlight_timer();
void reset_backlight_alarm();

/**
 * @brief Checks if a DMA transfer is currently in progress.
 * @return true if busy, false if ready.
 */
bool display_is_busy();

void display_wait_ready();

/**
 * @brief Draws a single character using the internal 5x7 font.
 */
void display_draw_char(int x, int y, char c, uint16_t color, uint16_t bg, uint8_t size);

/**
 * @brief Draws a string to the back buffer.
 */
void display_draw_string(int x, int y, const char* str, uint16_t color, uint16_t bg, uint8_t size);

/**
 * @brief Draws a string with word-wrapping and vertical scrolling support.
 * @param max_width Maximum width in pixels before wrapping.
 * @param line_height Height of each line in pixels.
 * @param scroll_offset Vertical offset for the viewport.
 */
void display_draw_string_with_scroll(int x, int y, const char* str, uint16_t color, uint16_t bg, uint8_t size, int max_width, int line_height, int scroll_offset);

#endif // DISPLAY_H
