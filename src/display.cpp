#include "display.h"
#include "hardware/gpio.h"
#include <string.h>

// Global framebuffer (40,960 bytes)
static uint16_t framebuffer[LCD_WIDTH * LCD_HEIGHT];

// Internal functions for ST7735 communication
static void write_command(uint8_t cmd) {
    gpio_put(PIN_DC, 0);
    gpio_put(PIN_CS, 0);
    spi_write_blocking(SPI_PORT, &cmd, 1);
    gpio_put(PIN_CS, 1);
}

static void write_data(uint8_t data) {
    gpio_put(PIN_DC, 1);
    gpio_put(PIN_CS, 0);
    spi_write_blocking(SPI_PORT, &data, 1);
    gpio_put(PIN_CS, 1);
}

static void set_window(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
    // Hardware offsets for 1.8" ST7735
    uint8_t x_offset = 2;
    uint8_t y_offset = 1;

    write_command(0x2A); // CASET
    write_data(0x00);
    write_data(x0 + x_offset);
    write_data(0x00);
    write_data(x1 + x_offset);

    write_command(0x2B); // RASET
    write_data(0x00);
    write_data(y0 + y_offset);
    write_data(0x00);
    write_data(y1 + y_offset);

    write_command(0x2C); // RAMWR
}

void display_init() {
    // Initialize SPI
    spi_init(SPI_PORT, 12 * 1000 * 1000);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    // Initialize Control Pins
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    gpio_init(PIN_DC);
    gpio_set_dir(PIN_DC, GPIO_OUT);

    gpio_init(PIN_RESET);
    gpio_set_dir(PIN_RESET, GPIO_OUT);

    gpio_init(PIN_BL);
    gpio_set_dir(PIN_BL, GPIO_OUT);
    gpio_put(PIN_BL, 1);

    // Hardware Reset
    gpio_put(PIN_RESET, 1);
    sleep_ms(10);
    gpio_put(PIN_RESET, 0);
    sleep_ms(10);
    gpio_put(PIN_RESET, 1);
    sleep_ms(120);

    write_command(0x01); // Software Reset
    sleep_ms(120);
    write_command(0x11); // Sleep Out
    sleep_ms(120);
    write_command(0x3A); // Color Mode
    write_data(0x05);    // 16-bit
    write_command(0x20); // Inversion Off
    write_command(0x36); // MADCTL
    write_data(0xC0);    // RGB mode
    write_command(0x29); // Display On
    sleep_ms(100);
}

void display_clear(uint16_t color) {
    for (int i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++) {
        framebuffer[i] = color;
    }
}

void display_draw_pixel(int x, int y, uint16_t color) {
    if (x < 0 || x >= LCD_WIDTH || y < 0 || y >= LCD_HEIGHT) return;
    framebuffer[y * LCD_WIDTH + x] = color;
}

void display_draw_rect(int x, int y, int w, int h, uint16_t color) {
    for (int j = y; j < y + h; j++) {
        for (int i = x; i < x + w; i++) {
            display_draw_pixel(i, j, color);
        }
    }
}

void display_flush() {
    set_window(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    
    gpio_put(PIN_DC, 1);
    gpio_put(PIN_CS, 0);
    
    // Send entire framebuffer. 
    // ST7735 expects big-endian (MSB first) for 16-bit colors.
    // We need to swap bytes if the system is little-endian (RP2040 is).
    
    static uint8_t swap_buffer[LCD_WIDTH * 2];
    for (int y = 0; y < LCD_HEIGHT; y++) {
        for (int x = 0; x < LCD_WIDTH; x++) {
            uint16_t pixel = framebuffer[y * LCD_WIDTH + x];
            swap_buffer[x * 2] = (pixel >> 8) & 0xFF;
            swap_buffer[x * 2 + 1] = pixel & 0xFF;
        }
        spi_write_blocking(SPI_PORT, swap_buffer, sizeof(swap_buffer));
    }
    
    gpio_put(PIN_CS, 1);
}
