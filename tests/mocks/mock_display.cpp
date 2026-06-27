#include "display.h"
#include "font.h"
#include <string.h>

uint16_t mock_backlight_level = 0;
bool mock_display_busy = false;

static uint16_t buffer_0[LCD_WIDTH * LCD_HEIGHT];
static uint16_t buffer_1[LCD_WIDTH * LCD_HEIGHT];
uint16_t *front_buffer = buffer_0;
uint16_t *back_buffer = buffer_1;

void display_init() {
    mock_backlight_level = 65535;
    mock_display_busy = false;
    memset(buffer_0, 0, sizeof(buffer_0));
    memset(buffer_1, 0, sizeof(buffer_1));
}

void display_set_backlight(uint16_t level) {
    mock_backlight_level = level;
}

void display_clear(uint16_t color) {
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

void display_draw_hollow_rect(int x, int y, int w, int h, uint16_t color) {
    display_draw_rect(x, y, w, 1, color);           // Top
    display_draw_rect(x, y + h - 1, w, 1, color);   // Bottom
    display_draw_rect(x, y, 1, h, color);           // Left
    display_draw_rect(x + w - 1, y, 1, h, color);   // Right
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
    return mock_display_busy;
}

void display_flush_async() {
    uint16_t *tmp = front_buffer;
    front_buffer = back_buffer;
    back_buffer = tmp;
}

void display_init_backlight_timer() {}
void reset_backlight_alarm() {}
void display_wait_ready() {}
