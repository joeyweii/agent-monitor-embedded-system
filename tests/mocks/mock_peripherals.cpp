#include "pico/time.h"
#include "hardware/gpio.h"
#include "tusb.h"

// Define the global mock variables
uint64_t mock_absolute_time_us = 0;

bool mock_gpio_states[32] = {false};
uint mock_gpio_dirs[32] = {0};
bool mock_gpio_pullups[32] = {false};
gpio_irq_callback_t mock_gpio_irq_callback = nullptr;
uint32_t mock_gpio_irq_events = 0;

std::vector<uint8_t> mock_cdc_rx_data;
size_t mock_cdc_rx_idx = 0;
