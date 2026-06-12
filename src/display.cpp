#include "display.h"
#include "hardware/gpio.h"
#include "hardware/dma.h"
#include <string.h>

// Double Buffering Allocation (80KB total)
static uint16_t buffer_0[LCD_WIDTH * LCD_HEIGHT];
static uint16_t buffer_1[LCD_WIDTH * LCD_HEIGHT];

static uint16_t *front_buffer = buffer_0; // Being sent to LCD via DMA
static uint16_t *back_buffer = buffer_1;  // Being drawn into by CPU

static int dma_chan;
static dma_channel_config dma_config;

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
}

void display_clear(uint16_t color) {
    // In 16-bit SPI mode, we store colors as-is (no manual swap needed)
    for (int i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++) {
        back_buffer[i] = color;
    }
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

bool display_is_busy() {
    return dma_channel_is_busy(dma_chan);
}

void display_wait_ready() {
    while (display_is_busy()) {
        tight_loop_contents();
    }
}

void display_flush_async() {
    display_wait_ready();

    // Swap buffers
    uint16_t *tmp = front_buffer;
    front_buffer = back_buffer;
    back_buffer = tmp;

    // Prepare display for transfer
    set_window(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    gpio_put(PIN_DC, 1);
    gpio_put(PIN_CS, 0);

    // Start DMA transfer from front_buffer
    dma_channel_configure(
        dma_chan,
        &dma_config,
        &spi_get_hw(SPI_PORT)->dr,        // Write to SPI TX FIFO
        front_buffer,                     // Read from front buffer
        LCD_WIDTH * LCD_HEIGHT,           // Number of 16-bit transfers
        true                              // Start immediately
    );
}
