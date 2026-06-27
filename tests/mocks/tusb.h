#ifndef MOCK_TUSB_H
#define MOCK_TUSB_H

#include <stdint.h>
#include <stdbool.h>
#include <vector>

extern std::vector<uint8_t> mock_cdc_rx_data;
extern size_t mock_cdc_rx_idx;

inline bool tud_cdc_available() {
    return mock_cdc_rx_idx < mock_cdc_rx_data.size();
}

inline uint8_t tud_cdc_read_char() {
    if (mock_cdc_rx_idx < mock_cdc_rx_data.size()) {
        return mock_cdc_rx_data[mock_cdc_rx_idx++];
    }
    return 0;
}

// TinyUSB RX Callback function prototype
extern "C" void tud_cdc_rx_cb(uint8_t itf);

#endif // MOCK_TUSB_H
