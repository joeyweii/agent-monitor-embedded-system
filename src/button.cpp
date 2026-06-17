#include "button.h"
#include "display.h"
#include "ui.h"
#include "hardware/gpio.h"

volatile bool button_event_flag = false;
volatile uint target_button_gpio = 0;
static bool led_state = false;

void gpio_irq_handler(uint gpio, uint32_t events) {
    target_button_gpio = gpio;
    button_event_flag = true;
}

void buttons_init() {
    uint buttons[] = {BTN_PREV, BTN_NEXT, BTN_SELECT};
    for(int i=0; i<3; i++) {
        gpio_init(buttons[i]);
        gpio_set_dir(buttons[i], GPIO_IN);
        gpio_pull_up(buttons[i]);
        // Set interrupt for falling edge (press)
        gpio_set_irq_enabled_with_callback(buttons[i], GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    }

    // Initialize LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    led_set(true);
}

bool button_is_pressed(uint pin) {
    return gpio_get(pin) == 0;
}

void handle_button_event(uint gpio) {
    if (absolute_time_diff_us(last_button_time, get_absolute_time()) < 200000) return;

    ui_wake_up();
    reset_backlight_alarm();

    if (gpio == BTN_PREV) {
        last_button_time = get_absolute_time();
        if (current_state == STATE_LIST) {
            int next_idx = selected_idx;
            for (int i = 0; i < MAX_AGENTS; i++) {
                next_idx = (next_idx + MAX_AGENTS - 1) % MAX_AGENTS;
                if (protocol_get_agent(next_idx)->is_active) {
                    selected_idx = next_idx;
                    break;
                }
            }
        } else if (current_state == STATE_DETAIL) {
            if (scroll_offset > 0) scroll_offset -= UI_DETAIL_SCROLL_STEP;
        }
    } else if (gpio == BTN_NEXT) {
        last_button_time = get_absolute_time();
        if (current_state == STATE_LIST) {
            int next_idx = selected_idx;
            for (int i = 0; i < MAX_AGENTS; i++) {
                next_idx = (next_idx + 1) % MAX_AGENTS;
                if (protocol_get_agent(next_idx)->is_active) {
                    selected_idx = next_idx;
                    break;
                }
            }
        } else if (current_state == STATE_DETAIL) {
            scroll_offset += UI_DETAIL_SCROLL_STEP;
        }
    } else if (gpio == BTN_SELECT) {
        last_button_time = get_absolute_time();
        if (current_state == STATE_LIST) {
            if (protocol_get_agent(selected_idx)->is_active) {
                current_state = STATE_DETAIL;
                scroll_offset = 0;
            }
        } else if (current_state == STATE_DETAIL) {
            current_state = STATE_LIST;
        }
    }
}

void led_set(bool state) {
    gpio_put(LED_PIN, state);
}

void led_toggle() {
    led_state = !led_state;
    gpio_put(LED_PIN, led_state);
}
