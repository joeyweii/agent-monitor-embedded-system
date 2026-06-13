#include <stdio.h>
#include "pico/stdlib.h"
#include "display.h"
#include "button.h"
#include "protocol.h"

int main() {
    stdio_init_all();

    // Initialize Modules
    display_init();
    buttons_init();
    protocol_init();

    // Initial Screen
    display_clear(COLOR_BLACK);
    display_draw_string(5, 5, "AGENT MONITOR", COLOR_YELLOW, COLOR_BLACK, 1);
    display_flush_async();

    while (true) {
        led_toggle();

        // 1. Process Communication
        protocol_update();

        // 2. Check for updates and render
        bool needs_refresh = false;
        for (int i = 0; i < MAX_AGENTS; i++) {
            AgentData* agent = protocol_get_agent(i);
            if (agent->is_active && agent->is_dirty) {
                needs_refresh = true;
                agent->is_dirty = false;
            }
        }

        if (needs_refresh) {
            display_clear(COLOR_BLACK);
            display_draw_string(5, 5, "AGENT MONITOR", COLOR_YELLOW, COLOR_BLACK, 1);
            
            // Draw Agents
            int y_offset = 25;
            for (int i = 0; i < MAX_AGENTS; i++) {
                AgentData* agent = protocol_get_agent(i);
                if (agent->is_active) {
                    char buffer[64];
                    snprintf(buffer, sizeof(buffer), "A%d: %s", agent->id, agent->name);
                    display_draw_string(5, y_offset, buffer, COLOR_WHITE, COLOR_BLACK, 1);
                    display_draw_string(5, y_offset + 10, agent->status, COLOR_GREEN, COLOR_BLACK, 1);
                    display_draw_string(5, y_offset + 20, agent->message, COLOR_BLUE, COLOR_BLACK, 1);
                    y_offset += 40;
                }
            }
            display_flush_async();
        }

        // 3. Process Input
        if (button_is_pressed(BTN_SELECT)) {
            display_clear(COLOR_WHITE);
            display_draw_string(20, 70, "APPROVED!", COLOR_BLACK, COLOR_WHITE, 1);
            display_flush_async();
            sleep_ms(500);
            
            // Force redraw pattern
            display_clear(COLOR_BLACK);
            display_flush_async();
        }

        sleep_ms(50);
    }

    return 0;
}
