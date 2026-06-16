#include "ui.h"
#include "display.h"
#include "button.h"
#include "pico/time.h"
#include "hardware/timer.h"
#include <stdio.h>
#include <string.h>

UIState current_state = STATE_LIST;
int selected_idx = 0;
int scroll_offset = 0;
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
        add_repeating_timer_ms(100, ui_timer_callback, NULL, &animation_timer);
        timer_running = true;
    } else if (!any_running && timer_running) {
        cancel_repeating_timer(&animation_timer);
        timer_running = false;
    }
}

void ui_init() {
    current_state = STATE_LIST;
    selected_idx = 0;
    scroll_offset = 0;
    frame_counter = 0;
    last_button_time = get_absolute_time();
}

void ui_draw_header() {
    display_draw_rect(0, 0, LCD_WIDTH, 15, COLOR_YELLOW);
    display_draw_string(25, 4, "AGENT MONITOR", COLOR_BLACK, COLOR_YELLOW, 1);
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

void ui_update() {
    ui_update_animation_timer(); // Manage animation timer dynamically
    
    display_clear(COLOR_BLACK);
    ui_draw_header();

    if (current_state == STATE_LIST) {
        for (int i = 0; i < MAX_AGENTS; i++) {
            AgentData* agent = protocol_get_agent(i);
            if (agent->is_active) {
                char buffer[64];
                snprintf(buffer, sizeof(buffer), "%c %d: %s", (i == selected_idx) ? '>' : ' ', agent->id, agent->name);
                uint16_t color = (i == selected_idx) ? COLOR_WHITE : COLOR_GRAY;
                display_draw_string(5, 25 + (i * 18), buffer, color, COLOR_BLACK, 1);
                
                // Draw Icon based on status
                if (agent->status == AGENT_STATUS_RUNNING) {
                   int frame = (frame_counter) % 4;
                   int x_pos = 105;
                   int y_pos = 25 + (i * 18);
                   switch (frame) {
                        case 0: display_draw_rect(x_pos, y_pos, 5, 2, COLOR_CYAN); break;
                        case 1: display_draw_rect(x_pos+3, y_pos, 2, 5, COLOR_CYAN); break;
                        case 2: display_draw_rect(x_pos, y_pos+3, 5, 2, COLOR_CYAN); break;
                        case 3: display_draw_rect(x_pos, y_pos, 2, 5, COLOR_CYAN); break;
                   }
                } else {
                    ui_draw_icon(105, 25 + (i * 18), agent->status);
                }
            }
        }
        frame_counter++;
    } else if (current_state == STATE_DETAIL) {
        AgentData* agent = protocol_get_agent(selected_idx);
        
        // Draw Card Border (Hollow)
        display_draw_rect(2, 20, LCD_WIDTH-4, 1, COLOR_GRAY); // Top
        display_draw_rect(2, LCD_HEIGHT-5, LCD_WIDTH-4, 1, COLOR_GRAY); // Bottom
        display_draw_rect(2, 20, 1, LCD_HEIGHT-25, COLOR_GRAY); // Left
        display_draw_rect(LCD_WIDTH-2, 20, 1, LCD_HEIGHT-25, COLOR_GRAY); // Right
        
        display_draw_string(10, 25, agent->name, COLOR_ROSE, COLOR_BLACK, 1);
        display_draw_rect(5, 35, LCD_WIDTH-10, 1, COLOR_GRAY); // Separator
        
        display_draw_string(10, 45, "Status:", COLOR_GRAY, COLOR_BLACK, 1);
        display_draw_string(60, 45, status_to_string(agent->status), get_status_color(agent->status), COLOR_BLACK, 1);
        
        display_draw_rect(5, 58, LCD_WIDTH-10, 1, COLOR_GRAY); // Separator
        
        display_draw_string_with_scroll(10, 70, agent->message, COLOR_WHITE, COLOR_BLACK, 1, LCD_WIDTH - 20, 10, scroll_offset);
    }

    display_flush_async();
}
