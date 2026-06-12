#include <stdio.h>
#include "pico/stdlib.h"
#include "display.h"
#include "button.h"

int main() {
    stdio_init_all();

    // Initialize Modules
    display_init();
    buttons_init();

    // Visual Test Pattern: Draw a border and a center square
    display_clear(COLOR_BLACK);
    
    // Border
    display_draw_rect(0, 0, LCD_WIDTH, 2, COLOR_RED);             // Top
    display_draw_rect(0, LCD_HEIGHT - 2, LCD_WIDTH, 2, COLOR_RED); // Bottom
    display_draw_rect(0, 0, 2, LCD_HEIGHT, COLOR_RED);             // Left
    display_draw_rect(LCD_WIDTH - 2, 0, 2, LCD_HEIGHT, COLOR_RED); // Right

    // Center Square
    display_draw_rect(LCD_WIDTH/2 - 10, LCD_HEIGHT/2 - 10, 20, 20, COLOR_GREEN);

    display_flush();

    static uint8_t color_idx = 0;
    uint16_t colors[] = {COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_YELLOW};

    while (true) {
        led_toggle();

        bool changed = false;

        if (button_is_pressed(BTN_PREV)) {
            color_idx = (color_idx + 3) % 4;
            changed = true;
        } else if (button_is_pressed(BTN_NEXT)) {
            color_idx = (color_idx + 1) % 4;
            changed = true;
        } else if (button_is_pressed(BTN_SELECT)) {
            display_clear(COLOR_WHITE);
            display_flush();
            sleep_ms(250);
            // Redraw pattern after white flash
            display_clear(COLOR_BLACK);
            display_draw_rect(0, 0, LCD_WIDTH, 2, COLOR_RED);
            display_draw_rect(0, LCD_HEIGHT - 2, LCD_WIDTH, 2, COLOR_RED);
            display_draw_rect(0, 0, 2, LCD_HEIGHT, COLOR_RED);
            display_draw_rect(LCD_WIDTH - 2, 0, 2, LCD_HEIGHT, COLOR_RED);
            display_draw_rect(LCD_WIDTH/2 - 10, LCD_HEIGHT/2 - 10, 20, 20, colors[color_idx]);
            display_flush();
        }

        if (changed) {
            // Update the center square color based on selection
            display_draw_rect(LCD_WIDTH/2 - 10, LCD_HEIGHT/2 - 10, 20, 20, colors[color_idx]);
            display_flush();
            sleep_ms(250);
        }

        sleep_ms(50);
    }

    return 0;
}
