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
    
    // Draw Text
    display_draw_string(5, 20, "AGENT MONITOR", COLOR_YELLOW, COLOR_BLACK, 1);
    display_draw_string(10, 40, "Agent A: RUNNING", COLOR_GREEN, COLOR_BLACK, 1);
    display_draw_string(10, 60, "Agent B: IDLE", COLOR_BLUE, COLOR_BLACK, 1);

    display_flush_async();

    static uint8_t color_idx = 0;
    uint16_t colors[] = {COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_YELLOW};

    while (true) {
        led_toggle();

        // 1. Wait for previous DMA transfer to complete before drawing new frame
        display_wait_ready();

        // 2. Process Input
        if (button_is_pressed(BTN_SELECT)) {
            display_clear(COLOR_WHITE);
            display_draw_string(20, 70, "APPROVED!", COLOR_BLACK, COLOR_WHITE, 1);
            display_flush_async();
            sleep_ms(500);
            
            // Redraw pattern
            display_clear(COLOR_BLACK);
            display_draw_string(5, 20, "AGENT MONITOR", COLOR_YELLOW, COLOR_BLACK, 1);
            display_draw_string(10, 40, "Agent A: RUNNING", COLOR_GREEN, COLOR_BLACK, 1);
            display_draw_string(10, 60, "Agent B: IDLE", COLOR_BLUE, COLOR_BLACK, 1);
            display_flush_async();
        }

        sleep_ms(100);
    }

    return 0;
}
