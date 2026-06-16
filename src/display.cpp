#include "display.h"
#include "font.h"
#include "hardware/gpio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"

// Double Buffering Allocation (80KB total)
static uint16_t buffer_0[LCD_WIDTH * LCD_HEIGHT];
static uint16_t buffer_1[LCD_WIDTH * LCD_HEIGHT];

static uint16_t *front_buffer = buffer_0; 
static uint16_t *back_buffer = buffer_1;  

static int dma_chan;
static dma_channel_config dma_config;

static volatile bool dma_transfer_complete = true;

// DMA Interrupt Handler
void dma_handler() {
    if (dma_hw->ints0 & (1u << dma_chan)) {
        dma_hw->ints0 = 1u << dma_chan; // Clear the interrupt
        
        // Wait for SPI FIFO to be empty and last bit to be sent
        while (spi_is_busy(SPI_PORT));
        
        dma_transfer_complete = true;
        gpio_put(PIN_CS, 1); // Release CS after transfer completes
    }
}

// Internal functions for ST7735 communication
static void write_command(uint8_t cmd) {
    spi_set_format(SPI_PORT, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_put(PIN_DC, 0);
    gpio_put(PIN_CS, 0);
    spi_write_blocking(SPI_PORT, &cmd, 1);
    gpio_put(PIN_CS, 1);
    spi_set_format(SPI_PORT, 16, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
}

static void write_data(uint8_t data) {
    spi_set_format(SPI_PORT, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_put(PIN_DC, 1);
    gpio_put(PIN_CS, 0);
    spi_write_blocking(SPI_PORT, &data, 1);
    gpio_put(PIN_CS, 1);
    spi_set_format(SPI_PORT, 16, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
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
    // Initialize SPI at high speed (24MHz target)
    spi_init(SPI_PORT, 24 * 1000 * 1000);
    
    // Default to 16-bit format for DMA efficiency
    spi_set_format(SPI_PORT, 16, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

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

    // ST7735 Initialization Sequence
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

    // Initialize DMA
    dma_chan = dma_claim_unused_channel(true);
    dma_config = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_16);
    channel_config_set_dreq(&dma_config, spi_get_index(SPI_PORT) ? DREQ_SPI1_TX : DREQ_SPI0_TX);
    channel_config_set_read_increment(&dma_config, true);
    channel_config_set_write_increment(&dma_config, false);

    dma_channel_set_irq0_enabled(dma_chan, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);
}

void display_clear(uint16_t color) {
    for (int i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++) back_buffer[i] = color;
}

void display_draw_pixel(int x, int y, uint16_t color) {
    if (x < 0 || x >= LCD_WIDTH || y < 0 || y >= LCD_HEIGHT) return;
    back_buffer[y * LCD_WIDTH + x] = color;
}

void display_draw_rect(int x, int y, int w, int h, uint16_t color) {
    for (int j = y; j < y + h; j++) {
        if (j < 0 || j >= LCD_HEIGHT) continue;
        for (int i = x; i < x + w; i++) {
            if (i < 0 || i >= LCD_WIDTH) continue;
            back_buffer[j * LCD_WIDTH + i] = color;
        }
    }
}

void display_draw_char(int x, int y, char c, uint16_t color, uint16_t bg, uint8_t size) {
    if (c < 32 || c > 126) c = '?';
    const uint8_t *glyph = font_5x7[c - 32];
    for (int i = 0; i < 5; i++) {
        uint8_t line = glyph[i];
        for (int j = 0; j < 8; j++) {
            if (line & (1 << j)) {
                if (size == 1) display_draw_pixel(x + i, y + j, color);
                else display_draw_rect(x + i * size, y + j * size, size, size, color);
            } else if (bg != color) {
                if (size == 1) display_draw_pixel(x + i, y + j, bg);
                else display_draw_rect(x + i * size, y + j * size, size, size, bg);
            }
        }
    }
}

void display_draw_string(int x, int y, const char* str, uint16_t color, uint16_t bg, uint8_t size) {
    int cur_x = x;
    while (*str) {
        display_draw_char(cur_x, y, *str++, color, bg, size);
        cur_x += 6 * size;
    }
}

void display_draw_string_with_scroll(int x, int y, const char* str, uint16_t color, uint16_t bg, uint8_t size, int max_width, int line_height, int scroll_offset) {
    int cur_x = x;
    int cur_y = y;
    int char_w = 6 * size;

    while (*str) {
        if (cur_x + char_w > x + max_width) {
            cur_x = x;
            cur_y += line_height;
        }
        if (cur_y >= y + scroll_offset && cur_y < y + scroll_offset + (line_height * 8)) {
            display_draw_char(cur_x, cur_y - scroll_offset, *str, color, bg, size);
        }
        cur_x += char_w;
        str++;
    }
}

bool display_is_busy() {
    return !dma_transfer_complete;
}

void display_flush_async() {
    if (display_is_busy()) return; // Non-blocking: skip if busy

    dma_transfer_complete = false;


    // Swap buffers
    uint16_t *tmp = front_buffer;
    front_buffer = back_buffer;
    back_buffer = tmp;

    set_window(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    gpio_put(PIN_DC, 1);
    gpio_put(PIN_CS, 0);

    dma_channel_configure(
        dma_chan,
        &dma_config,
        &spi_get_hw(SPI_PORT)->dr,        // Write to SPI TX FIFO
        front_buffer,                     // Read from front buffer
        LCD_WIDTH * LCD_HEIGHT,           // Number of 16-bit transfers
        true                              // Start immediately
    );
}
