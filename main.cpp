#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"

// Define Pins
#define PIN_SCK   18
#define PIN_MOSI  19
#define PIN_CS    17
#define PIN_DC    20
#define PIN_RESET 21
#define PIN_BL    22
#define LED_PIN   25

#define BTN_PREV   26  // GP26 = Pin 31 (Row 22/24)
#define BTN_NEXT   27  // GP27 = Pin 32 (Row 25/27)
#define BTN_SELECT 28  // GP28 = Pin 34 (Row 28/30)

// SPI Instance
#define SPI_PORT spi0

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

// Write a command to the display (DC Pin LOW)
void write_command(uint8_t cmd) {
    gpio_put(PIN_DC, 0); // Command Mode
    gpio_put(PIN_CS, 0); // Select Display
    spi_write_blocking(SPI_PORT, &cmd, 1);
    gpio_put(PIN_CS, 1); // Deselect Display
}

// Write a single byte of data to the display (DC Pin HIGH)
void write_data(uint8_t data) {
    gpio_put(PIN_DC, 1); // Data Mode
    gpio_put(PIN_CS, 0); // Select Display
    spi_write_blocking(SPI_PORT, &data, 1);
    gpio_put(PIN_CS, 1); // Deselect Display
}

// Set drawing window with offsets to fix edge noise
void set_window(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
    // Standard offsets for 1.8" ST7735
    write_command(0x2A); // Column Address Set (CASET)
    write_data(0x00);
    write_data(x0);
    write_data(0x00);
    write_data(x1);

    write_command(0x2B); // Row Address Set (RASET)
    write_data(0x00);
    write_data(y0);
    write_data(0x00);
    write_data(y1);

    write_command(0x2C); // Memory Write (RAMWR)
}

// Initialize the ST7735 screen
void st7735_init() {
    // Reset the display
    gpio_put(PIN_RESET, 1);
    sleep_ms(10);
    gpio_put(PIN_RESET, 0);
    sleep_ms(10);
    gpio_put(PIN_RESET, 1);
    sleep_ms(120);

    write_command(0x01); // Software Reset
    sleep_ms(120);

    write_command(0x11); // Sleep Out (Exit Sleep mode)
    sleep_ms(120);

    // Color Mode (3Ah): 16-bit color (0x05)
    write_command(0x3A);
    write_data(0x05);

    // FIX: Keep Display Inversion OFF (0x20)
    write_command(0x20); 

    // Memory Data Access Control (MADCTL)
    write_command(0x36);
    write_data(0xC0); // RGB color ordering

    // Display On
    write_command(0x29);
    sleep_ms(100);
}

// Fill screen with a solid color
void fill_screen(uint16_t color) {
    set_window(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    
    gpio_put(PIN_DC, 1);
    gpio_put(PIN_CS, 0);
    
    uint8_t line_buffer[LCD_WIDTH * 2];
    for (int x = 0; x < LCD_WIDTH; x++) {
        line_buffer[x * 2] = (color >> 8) & 0xFF;     // High byte
        line_buffer[x * 2 + 1] = color & 0xFF;         // Low byte
    }
    
    for (int y = 0; y < LCD_HEIGHT; y++) {
        spi_write_blocking(SPI_PORT, line_buffer, sizeof(line_buffer));
    }
    
    gpio_put(PIN_CS, 1);
}

int main() {
    stdio_init_all();

    spi_init(SPI_PORT, 12 * 1000 * 1000);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

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

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1);

    gpio_init(BTN_PREV);
    gpio_set_dir(BTN_PREV, GPIO_IN);
    gpio_pull_up(BTN_PREV);

    gpio_init(BTN_NEXT);
    gpio_set_dir(BTN_NEXT, GPIO_IN);
    gpio_pull_up(BTN_NEXT);

    gpio_init(BTN_SELECT);
    gpio_set_dir(BTN_SELECT, GPIO_IN);
    gpio_pull_up(BTN_SELECT);

    st7735_init();
    fill_screen(COLOR_YELLOW);

    static uint8_t color_idx = 3;
    bool led_state = true;
    while (true) {
        led_state = !led_state;
        gpio_put(LED_PIN, led_state);

        bool prev_val   = gpio_get(BTN_PREV);
        bool next_val   = gpio_get(BTN_NEXT);
        bool select_val = gpio_get(BTN_SELECT);

        if (prev_val == 0) {
            color_idx = (color_idx + 3) % 4;
            switch (color_idx) {
                case 0: fill_screen(COLOR_RED);    break;
                case 1: fill_screen(COLOR_GREEN);  break;
                case 2: fill_screen(COLOR_BLUE);   break;
                case 3: fill_screen(COLOR_YELLOW); break;
            }
            sleep_ms(250);
        } else if (next_val == 0) {
            color_idx = (color_idx + 1) % 4;
            switch (color_idx) {
                case 0: fill_screen(COLOR_RED);    break;
                case 1: fill_screen(COLOR_GREEN);  break;
                case 2: fill_screen(COLOR_BLUE);   break;
                case 3: fill_screen(COLOR_YELLOW); break;
            }
            sleep_ms(250);
        } else if (select_val == 0) {
            fill_screen(COLOR_WHITE);
            sleep_ms(250);
        }
        sleep_ms(100);
    }
    return 0;
}
