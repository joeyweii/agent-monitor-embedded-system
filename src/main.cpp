#include <stdio.h>
#include "pico/stdlib.h"
#include "display.h"
#include "button.h"

int main() {
    stdio_init_all();

    // Initialize Modules
    display_init();
    buttons_init();

    static uint8_t color_idx = 0;
    uint16_t colors[] = {COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_YELLOW};

    uint32_t last_button_time = 0;
    bool is_dirty = true; // Start true to render initial frame

    while (true) {
        led_toggle();

        uint32_t current_time = to_ms_since_boot(get_absolute_time());

        // 1. Process Input (Non-blocking debounce)
        if (current_time - last_button_time > 200) {
            if (button_is_pressed(BTN_PREV)) {
                color_idx = (color_idx + 3) % 4;
                is_dirty = true;
                last_button_time = current_time;
            } else if (button_is_pressed(BTN_NEXT)) {
                color_idx = (color_idx + 1) % 4;
                is_dirty = true;
                last_button_time = current_time;
            } else if (button_is_pressed(BTN_SELECT)) {
                // Flash white logic
                display_wait_ready();
                display_clear(COLOR_WHITE);
                display_flush_async();
                sleep_ms(100); // Small block is okay for a special effect
                is_dirty = true; 
                last_button_time = current_time;
            }
        }

        // 2. Render and Flush only if needed AND display is ready
        if (is_dirty && !display_is_busy()) {
            // Draw to BACK BUFFER
            display_clear(COLOR_BLACK);
            
            // Draw standard border
            display_draw_rect(0, 0, LCD_WIDTH, 2, COLOR_RED);
            display_draw_rect(0, LCD_HEIGHT - 2, LCD_WIDTH, 2, COLOR_RED);
            display_draw_rect(0, 0, 2, LCD_HEIGHT, COLOR_RED);
            display_draw_rect(LCD_WIDTH - 2, 0, 2, LCD_HEIGHT, COLOR_RED);

            // Draw center square
            display_draw_rect(LCD_WIDTH/2 - 15, LCD_HEIGHT/2 - 15, 30, 30, colors[color_idx]);

            // Trigger Asynchronous Flush
            display_flush_async();
            
            is_dirty = false; // Reset dirty flag
        }

        // 3. Other system tasks can go here
        sleep_ms(10); 
    }

    return 0;
}
