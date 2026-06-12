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

    while (true) {
        led_toggle();

        // 1. Wait for previous DMA transfer to complete before drawing new frame
        display_wait_ready();

        // 2. Process Input
        bool changed = false;
        if (button_is_pressed(BTN_PREV)) {
            color_idx = (color_idx + 3) % 4;
            changed = true;
            sleep_ms(200); // Simple debounce
        } else if (button_is_pressed(BTN_NEXT)) {
            color_idx = (color_idx + 1) % 4;
            changed = true;
            sleep_ms(200); // Simple debounce
        } else if (button_is_pressed(BTN_SELECT)) {
            display_clear(COLOR_WHITE);
            display_flush_async();
            sleep_ms(200);
            continue;
        }

        // 3. Draw to BACK BUFFER
        display_clear(COLOR_BLACK);
        
        // Draw standard border
        display_draw_rect(0, 0, LCD_WIDTH, 2, COLOR_RED);
        display_draw_rect(0, LCD_HEIGHT - 2, LCD_WIDTH, 2, COLOR_RED);
        display_draw_rect(0, 0, 2, LCD_HEIGHT, COLOR_RED);
        display_draw_rect(LCD_WIDTH - 2, 0, 2, LCD_HEIGHT, COLOR_RED);

        // Draw animated/selectable center square
        display_draw_rect(LCD_WIDTH/2 - 15, LCD_HEIGHT/2 - 15, 30, 30, colors[color_idx]);

        // 4. Trigger Asynchronous Flush (DMA takes over here)
        display_flush_async();

        // The CPU is now free! We can do other things while the DMA sends the data.
        sleep_ms(10);
    }

    return 0;
}
