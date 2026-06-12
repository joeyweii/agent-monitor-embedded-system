#include "button.h"
#include "hardware/gpio.h"

static bool led_state = false;

void buttons_init() {
    // Initialize Buttons
    gpio_init(BTN_PREV);
    gpio_set_dir(BTN_PREV, GPIO_IN);
    gpio_pull_up(BTN_PREV);

    gpio_init(BTN_NEXT);
    gpio_set_dir(BTN_NEXT, GPIO_IN);
    gpio_pull_up(BTN_NEXT);

    gpio_init(BTN_SELECT);
    gpio_set_dir(BTN_SELECT, GPIO_IN);
    gpio_pull_up(BTN_SELECT);

    // Initialize LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    led_set(true);
}

bool button_is_pressed(uint pin) {
    // Active Low
    return gpio_get(pin) == 0;
}

void led_set(bool state) {
    led_state = state;
    gpio_put(LED_PIN, state);
}

void led_toggle() {
    led_state = !led_state;
    gpio_put(LED_PIN, led_state);
}
