#include "test_framework.h"
#include "ui.h"
#include "button.h"
#include "protocol.h"
#include "pico/time.h"
#include "hardware/gpio.h"
#include "tusb.h"
#include "display.h"

extern uint16_t *back_buffer;
extern uint16_t *front_buffer;

// Helper to press a button and advance time
static void press_button(uint gpio) {
    // Advance mock time by 250ms (250,000 us) to pass debouncing check (200ms)
    mock_absolute_time_us += 250000;
    handle_button_event(gpio);
}

void test_ui_init() {
    ui_init();
    ASSERT_EQ(current_state, STATE_LIST);
    ASSERT_EQ(selected_idx, -1);
    ASSERT_EQ(scroll_offset, 0);
}

void test_ui_carousel_cycling() {
    mock_absolute_time_us = 0;
    protocol_init();
    ui_init();
    
    // Add two active agents at IDs 1 and 4
    // Agent 1
    std::string packet1 = "SET:1:4:7:5:DONEAgent_AReady";
    mock_cdc_rx_data.clear();
    for (char c : packet1) mock_cdc_rx_data.push_back(c);
    mock_cdc_rx_idx = 0;
    tud_cdc_rx_cb(0);
    handle_protocol_event();
    
    // Agent 4
    std::string packet2 = "SET:4:4:7:5:DONEAgent_BReady";
    mock_cdc_rx_data.clear();
    for (char c : packet2) mock_cdc_rx_data.push_back(c);
    mock_cdc_rx_idx = 0;
    tud_cdc_rx_cb(0);
    handle_protocol_event();
    
    // selected_idx defaults to the first active agent added (ID 1)
    ASSERT_EQ((int)selected_idx, 1);
    
    // Press BTN_NEXT -> should cycle to ID 4
    press_button(BTN_NEXT);
    ASSERT_EQ((int)selected_idx, 4);
    
    // Press BTN_NEXT -> should wrap around to ID 1
    press_button(BTN_NEXT);
    ASSERT_EQ((int)selected_idx, 1);
    
    // Press BTN_PREV -> should cycle back to ID 4
    press_button(BTN_PREV);
    ASSERT_EQ((int)selected_idx, 4);
}

void test_ui_state_transition() {
    mock_absolute_time_us = 0;
    protocol_init();
    ui_init();
    
    // Add agent at ID 2
    std::string packet = "SET:2:4:7:5:DONEAgent_AReady";
    mock_cdc_rx_data.clear();
    for (char c : packet) mock_cdc_rx_data.push_back(c);
    mock_cdc_rx_idx = 0;
    tud_cdc_rx_cb(0);
    handle_protocol_event();
    
    ASSERT_EQ((int)selected_idx, 2);
    ASSERT_EQ(current_state, STATE_LIST);
    
    // Press BTN_SELECT -> should enter STATE_DETAIL
    press_button(BTN_SELECT);
    ASSERT_EQ(current_state, STATE_DETAIL);
    ASSERT_EQ(scroll_offset, 0);
    
    // Press BTN_SELECT -> should return to STATE_LIST
    press_button(BTN_SELECT);
    ASSERT_EQ(current_state, STATE_LIST);
}

void test_ui_scrolling() {
    mock_absolute_time_us = 0;
    protocol_init();
    ui_init();
    
    // Add agent
    std::string packet = "SET:2:4:7:5:DONEAgent_AReady";
    mock_cdc_rx_data.clear();
    for (char c : packet) mock_cdc_rx_data.push_back(c);
    mock_cdc_rx_idx = 0;
    tud_cdc_rx_cb(0);
    handle_protocol_event();
    
    // Enter detail view
    press_button(BTN_SELECT);
    ASSERT_EQ(current_state, STATE_DETAIL);
    ASSERT_EQ(scroll_offset, 0);
    
    // In detail view: BTN_NEXT scrolls down
    press_button(BTN_NEXT);
    ASSERT_EQ(scroll_offset, UI_DETAIL_SCROLL_STEP);
    
    press_button(BTN_NEXT);
    ASSERT_EQ(scroll_offset, UI_DETAIL_SCROLL_STEP * 2);
    
    // BTN_PREV scrolls up
    press_button(BTN_PREV);
    ASSERT_EQ(scroll_offset, UI_DETAIL_SCROLL_STEP);
    
    press_button(BTN_PREV);
    ASSERT_EQ(scroll_offset, 0);
    
    // Scrolling up at 0 should stay at 0
    press_button(BTN_PREV);
    ASSERT_EQ(scroll_offset, 0);
}

void test_ui_button_debouncing() {
    mock_absolute_time_us = 0;
    protocol_init();
    ui_init();
    
    // Add agent 1 & 2
    std::string packet1 = "SET:1:4:7:5:DONEAgent_AReady";
    mock_cdc_rx_data.clear();
    for (char c : packet1) mock_cdc_rx_data.push_back(c);
    mock_cdc_rx_idx = 0;
    tud_cdc_rx_cb(0);
    handle_protocol_event();
    
    std::string packet2 = "SET:2:4:7:5:DONEAgent_BReady";
    mock_cdc_rx_data.clear();
    for (char c : packet2) mock_cdc_rx_data.push_back(c);
    mock_cdc_rx_idx = 0;
    tud_cdc_rx_cb(0);
    handle_protocol_event();
    
    ASSERT_EQ((int)selected_idx, 1);
    
    // Simulate first press - advances time and transitions
    press_button(BTN_NEXT);
    ASSERT_EQ((int)selected_idx, 2);
    
    // Simulate a double press instantly (no time advancement)
    // Directly call handler since press_button helper advances time.
    handle_button_event(BTN_NEXT);
    
    // Should be ignored due to debouncing, selected_idx remains 2
    ASSERT_EQ((int)selected_idx, 2);
}

void test_ui_rendering() {
    protocol_init();
    ui_init();
    
    // Add an agent so the UI has something to render
    std::string packet = "SET:1:4:7:5:DONEAgent_AReady";
    mock_cdc_rx_data.clear();
    for (char c : packet) mock_cdc_rx_data.push_back(c);
    mock_cdc_rx_idx = 0;
    tud_cdc_rx_cb(0);
    handle_protocol_event();
    
    // Force a redraw
    ui_dirty_flag = true;
    ui_update();
    
    // After ui_update() calls display_flush_async(), buffers are swapped.
    // The just-rendered frame is now in front_buffer (what the display receives).
    // The top row (y=0) should be part of the yellow header bar.
    for (int x = 0; x < LCD_WIDTH; x++) {
        ASSERT_EQ(front_buffer[0 * LCD_WIDTH + x], COLOR_YELLOW);
    }
    
    // A pixel in the lower screen (e.g. y=80) should be COLOR_BLACK (background)
    ASSERT_EQ(front_buffer[80 * LCD_WIDTH + 10], COLOR_BLACK);
}

void register_ui_tests(std::vector<TestCase>& tests) {
    tests.push_back({"ui_init", test_ui_init});
    tests.push_back({"ui_carousel_cycling", test_ui_carousel_cycling});
    tests.push_back({"ui_state_transition", test_ui_state_transition});
    tests.push_back({"ui_scrolling", test_ui_scrolling});
    tests.push_back({"ui_button_debouncing", test_ui_button_debouncing});
    tests.push_back({"ui_rendering", test_ui_rendering});
}
