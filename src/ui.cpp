#include "ui.h"
#include "display.h"
#include "button.h"
#include "pico/time.h"
#include "hardware/timer.h"
#include <stdio.h>
#include <string.h>

UIState current_state = STATE_LIST;
int8_t selected_idx = -1;
int scroll_offset = 0;
bool is_asleep = false;
absolute_time_t last_button_time = {0};
static uint32_t frame_counter = 0;
volatile bool ui_dirty_flag = true;

static struct repeating_timer animation_timer;
static bool timer_running = false;

uint16_t get_status_color(AgentStatus status);

bool ui_timer_callback(struct repeating_timer *t) {
    ui_dirty_flag = true;
    return true; // Keep repeating
}

void ui_update_animation_timer() {
    bool any_running = false;
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (protocol_get_agent(i)->status == AGENT_STATUS_RUNNING) {
            any_running = true;
            break;
        }
    }

    if (any_running && !timer_running) {
        add_repeating_timer_ms(UI_ANIM_INTERVAL_MS, ui_timer_callback, NULL, &animation_timer);
        timer_running = true;
    } else if (!any_running && timer_running) {
        cancel_repeating_timer(&animation_timer);
        timer_running = false;
    }
}

void ui_init() {
    current_state = STATE_LIST;
    selected_idx = -1;
    scroll_offset = 0;
    frame_counter = 0;
    is_asleep = false;
    display_set_backlight(65535); // Full brightness
    last_button_time = get_absolute_time();
}

void ui_draw_header() {
    display_draw_rect(0, 0, LCD_WIDTH, UI_HEADER_HEIGHT, COLOR_YELLOW);
    display_draw_string(UI_HEADER_TEXT_X, UI_HEADER_TEXT_Y, "AGENT MONITOR", COLOR_BLACK, COLOR_YELLOW, 1);
}

uint16_t get_status_color(AgentStatus status) {
    switch (status) {
        case AGENT_STATUS_RUNNING: return COLOR_CYAN;
        case AGENT_STATUS_DONE:    return COLOR_GREEN;
        case AGENT_STATUS_ERROR:   return COLOR_RED;
        case AGENT_STATUS_INPUT:   return COLOR_YELLOW;
        default:                   return COLOR_WHITE;
    }
}

const char* status_to_string(AgentStatus status) {
    switch (status) {
        case AGENT_STATUS_RUNNING: return "RUNNING";
        case AGENT_STATUS_DONE:    return "DONE";
        case AGENT_STATUS_ERROR:   return "ERROR";
        case AGENT_STATUS_INPUT:   return "INPUT";
        default:                   return "UNKNOWN";
    }
}

void ui_draw_icon(int x, int y, AgentStatus status) {
    uint16_t color = get_status_color(status);
    
    if (status == AGENT_STATUS_DONE) { // Green Checkmark
        display_draw_pixel(x+2, y+5, color);
        display_draw_pixel(x+3, y+6, color);
        display_draw_pixel(x+4, y+7, color);
        display_draw_pixel(x+5, y+4, color);
        display_draw_pixel(x+6, y+3, color);
    } else if (status == AGENT_STATUS_ERROR) { // Red X
        display_draw_pixel(x+2, y+2, color);
        display_draw_pixel(x+3, y+3, color);
        display_draw_pixel(x+4, y+4, color);
        display_draw_pixel(x+2, y+4, color);
        display_draw_pixel(x+4, y+2, color);
    } else if (status == AGENT_STATUS_INPUT) { // Yellow ?
        display_draw_string(x, y, "?", color, COLOR_BLACK, 1);
    }
}

void ui_wake_up() {
    if (is_asleep) {
        is_asleep = false;
        display_set_backlight(65535);
        ui_dirty_flag = true;
    }
}

void ui_update() {
    if (is_asleep) return;

    ui_update_animation_timer(); // Manage animation timer dynamically
// ...


    display_clear(COLOR_BLACK);
    ui_draw_header();
// ...

    if (current_state == STATE_LIST) {
        for (int i = 0; i < MAX_AGENTS; i++) {
            AgentData* agent = protocol_get_agent(i);
            if (agent->is_active) {
                char buffer[64];
                snprintf(buffer, sizeof(buffer), "%c %d: %s", (i == selected_idx) ? '>' : ' ', agent->id, agent->name);
                uint16_t color = (i == selected_idx) ? COLOR_WHITE : COLOR_GRAY;
                display_draw_string(UI_MARGIN_X, UI_LIST_START_Y + (i * UI_LIST_LINE_HEIGHT), buffer, color, COLOR_BLACK, 1);
                
                // Draw Icon based on status
                if (agent->status == AGENT_STATUS_RUNNING) {
                   int frame = (frame_counter) % 4;
                   int x_pos = UI_LIST_STATUS_X;
                   int y_pos = UI_LIST_START_Y + (i * UI_LIST_LINE_HEIGHT);
                   switch (frame) {
                        case 0: display_draw_rect(x_pos, y_pos, UI_SPINNER_SIZE, 2, COLOR_CYAN); break;
                        case 1: display_draw_rect(x_pos + 3, y_pos, 2, UI_SPINNER_SIZE, COLOR_CYAN); break;
                        case 2: display_draw_rect(x_pos, y_pos + 3, UI_SPINNER_SIZE, 2, COLOR_CYAN); break;
                        case 3: display_draw_rect(x_pos, y_pos, 2, UI_SPINNER_SIZE, COLOR_CYAN); break;
                   }
                } else {
                    ui_draw_icon(UI_LIST_STATUS_X, UI_LIST_START_Y + (i * UI_LIST_LINE_HEIGHT), agent->status);
                }
            }
        }
        frame_counter++;
    } else if (current_state == STATE_DETAIL) {
        if (selected_idx < 0 || selected_idx >= MAX_AGENTS) {
            current_state = STATE_LIST;
            return;
        }
        AgentData* agent = protocol_get_agent(selected_idx);
        
        // Draw Card Border (Hollow)
        display_draw_hollow_rect(UI_DETAIL_CARD_X, UI_DETAIL_CARD_Y, LCD_WIDTH - (UI_DETAIL_CARD_X * 2), LCD_HEIGHT - (UI_DETAIL_CARD_Y + 5), COLOR_GRAY);
        
        display_draw_string(UI_DETAIL_MARGIN_X, UI_DETAIL_NAME_Y, agent->name, COLOR_ROSE, COLOR_BLACK, 1);
        display_draw_rect(UI_MARGIN_X, UI_DETAIL_SEP1_Y, LCD_WIDTH - (UI_MARGIN_X * 2), 1, COLOR_GRAY); // Separator
        
        display_draw_string(UI_DETAIL_MARGIN_X, UI_DETAIL_STATUS_Y, "Status:", COLOR_GRAY, COLOR_BLACK, 1);
        display_draw_string(UI_DETAIL_STATUS_VAL_X, UI_DETAIL_STATUS_Y, status_to_string(agent->status), get_status_color(agent->status), COLOR_BLACK, 1);
        
        display_draw_rect(UI_MARGIN_X, UI_DETAIL_SEP2_Y, LCD_WIDTH - (UI_MARGIN_X * 2), 1, COLOR_GRAY); // Separator
        
        display_draw_string_with_scroll(UI_DETAIL_MARGIN_X, UI_DETAIL_MSG_Y, agent->message, COLOR_WHITE, COLOR_BLACK, 1, LCD_WIDTH - (UI_DETAIL_MARGIN_X * 2), UI_DETAIL_LINE_HEIGHT, scroll_offset);
    }

    display_flush_async();
}
