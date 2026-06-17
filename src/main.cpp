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
        // Process UART Communication
        if (protocol_event_flag) {
            protocol_event_flag = false;
            handle_protocol_event();
            ui_dirty_flag = true;
        }

        // Process Button Event
        if (button_event_flag) {
            button_event_flag = false;
            handle_button_event(target_button_gpio);
            ui_dirty_flag = true;
        }

        // Render
        if (ui_dirty_flag && !display_is_busy()) {
            ui_dirty_flag = false;
            ui_update();
        }

        // Wait for interrupt (DMA, Serial, or Timer) safely
        uint32_t status = save_and_disable_interrupts();
        if (!protocol_event_flag && !button_event_flag && !ui_dirty_flag) {
            __wfi();
        }
        restore_interrupts(status);
    }

    return 0;
}

