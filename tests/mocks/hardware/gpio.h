#ifndef MOCK_HARDWARE_GPIO_H
#define MOCK_HARDWARE_GPIO_H

#include <stdint.h>
#include <stdbool.h>

#define GPIO_IN 0
#define GPIO_OUT 1
#define GPIO_IRQ_EDGE_FALL 1u

typedef unsigned int uint;
typedef void (*gpio_irq_callback_t)(uint gpio, uint32_t events);

extern bool mock_gpio_states[32];
extern uint mock_gpio_dirs[32];
extern bool mock_gpio_pullups[32];
extern gpio_irq_callback_t mock_gpio_irq_callback;
extern uint32_t mock_gpio_irq_events;

inline void gpio_init(uint gpio) {
    if (gpio < 32) {
        mock_gpio_states[gpio] = false;
        mock_gpio_dirs[gpio] = GPIO_IN;
        mock_gpio_pullups[gpio] = false;
    }
}

inline void gpio_set_dir(uint gpio, bool out) {
    if (gpio < 32) {
        mock_gpio_dirs[gpio] = out ? GPIO_OUT : GPIO_IN;
    }
}

inline void gpio_pull_up(uint gpio) {
    if (gpio < 32) {
        mock_gpio_pullups[gpio] = true;
    }
}

inline void gpio_put(uint gpio, bool value) {
    if (gpio < 32) {
        mock_gpio_states[gpio] = value;
    }
}

inline bool gpio_get(uint gpio) {
    if (gpio < 32) {
        return mock_gpio_states[gpio];
    }
    return false;
}

inline void gpio_set_irq_enabled_with_callback(uint gpio, uint32_t event_mask, bool enabled, gpio_irq_callback_t callback) {
    mock_gpio_irq_callback = callback;
    mock_gpio_irq_events = event_mask;
}

#endif // MOCK_HARDWARE_GPIO_H
