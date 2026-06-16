#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/sync.h"
#include "display.h"
#include "button.h"
#include "protocol.h"
#include "ui.h"

int main() {
    stdio_init_all();

    // Initialize Modules
    display_init();
    buttons_init();
    protocol_init();
    ui_init();

    while (true) {
        // Tracks whether the main loop is actively running
        led_toggle();

        // Process UART Communication
        if (protocol_event_flag) {
            handle_protocol_event();
            protocol_event_flag = false;
            ui_dirty_flag = true;
        }

        // Process Button Event
        if (button_event_flag) {
            handle_button_event(target_button_gpio);
            button_event_flag = false;
            ui_dirty_flag = true;
        }

        // Render
        if (ui_dirty_flag && !display_is_busy()) {
            ui_update();
            ui_dirty_flag = false;
        }

        // Wait for interrupt (DMA, Serial, or Timer)
        __wfi();
    }

    return 0;
}
