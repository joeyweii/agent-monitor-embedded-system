#include <stdio.h>
#include "pico/stdlib.h"
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
        led_toggle();

        // 1. Process Communication
        protocol_update();

        // 2. Process Input
        ui_handle_input();

        // 3. Render
        ui_update();

        sleep_ms(50);
    }

    return 0;
}
