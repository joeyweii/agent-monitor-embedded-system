#include "ui.h"
#include "display.h"
#include "button.h"
#include <stdio.h>
#include <string.h>

static UIState current_state = STATE_LIST;
static int selected_idx = 0;

void ui_init() {
    current_state = STATE_LIST;
    selected_idx = 0;
}

void ui_draw_header() {
    display_draw_rect(0, 0, LCD_WIDTH, 15, COLOR_YELLOW);
    display_draw_string(25, 4, "AGENT MONITOR", COLOR_BLACK, COLOR_YELLOW, 1);
}

void ui_draw_icon(int x, int y, int type) {
    if (type == 0) { // DONE (Green Checkmark)
        display_draw_pixel(x+2, y+5, COLOR_GREEN);
        display_draw_pixel(x+3, y+6, COLOR_GREEN);
        display_draw_pixel(x+4, y+7, COLOR_GREEN);
        display_draw_pixel(x+5, y+4, COLOR_GREEN);
        display_draw_pixel(x+6, y+3, COLOR_GREEN);
    } else if (type == 1) { // ERROR (Red X)
        display_draw_pixel(x+2, y+2, COLOR_RED);
        display_draw_pixel(x+3, y+3, COLOR_RED);
        display_draw_pixel(x+4, y+4, COLOR_RED);
        display_draw_pixel(x+2, y+4, COLOR_RED);
        display_draw_pixel(x+4, y+2, COLOR_RED);
    } else if (type == 2) { // INPUT (Yellow Question Mark)
        display_draw_string(x, y, "?", COLOR_YELLOW, COLOR_BLACK, 1);
    }
}

void ui_update() {
    display_clear(COLOR_BLACK);
    ui_draw_header();

    if (current_state == STATE_LIST) {
        for (int i = 0; i < MAX_AGENTS; i++) {
            AgentData* agent = protocol_get_agent(i);
            if (agent->is_active) {
                char buffer[64];
                // Removed 'A' prefix
                snprintf(buffer, sizeof(buffer), "%c %d: %s", (i == selected_idx) ? '>' : ' ', agent->id, agent->name);
                uint16_t color = (i == selected_idx) ? COLOR_WHITE : COLOR_GRAY;
                display_draw_string(5, 25 + (i * 20), buffer, color, COLOR_BLACK, 1);
                
                // Draw Icon based on status
                if (strcmp(agent->status, "RUNNING") == 0) {
                   // Placeholder for animation in next step
                   display_draw_rect(100, 25 + (i * 20), 5, 5, COLOR_BLUE);
                } else if (strcmp(agent->status, "DONE") == 0) {
                    ui_draw_icon(100, 25 + (i * 20), 0);
                } else if (strcmp(agent->status, "ERROR") == 0) {
                    ui_draw_icon(100, 25 + (i * 20), 1);
                } else if (strcmp(agent->status, "INPUT") == 0) {
                    ui_draw_icon(100, 25 + (i * 20), 2);
                }
            }
        }
    } else if (current_state == STATE_DETAIL) {
        AgentData* agent = protocol_get_agent(selected_idx);
        display_draw_string(5, 25, agent->name, COLOR_YELLOW, COLOR_BLACK, 1);
        display_draw_string(5, 45, agent->status, COLOR_GREEN, COLOR_BLACK, 1);
        display_draw_string(5, 65, agent->message, COLOR_BLUE, COLOR_BLACK, 1);
    } else if (current_state == STATE_ACTION) {
        display_draw_string(20, 70, "CONFIRM ACTION?", COLOR_RED, COLOR_BLACK, 1);
    }

    display_flush_async();
}

void ui_handle_input() {
    if (button_is_pressed(BTN_PREV)) {
        if (current_state == STATE_LIST) {
            selected_idx = (selected_idx + MAX_AGENTS - 1) % MAX_AGENTS;
        } else if (current_state == STATE_DETAIL) {
            current_state = STATE_LIST;
        }
        sleep_ms(200);
    } else if (button_is_pressed(BTN_NEXT)) {
        if (current_state == STATE_LIST) {
            selected_idx = (selected_idx + 1) % MAX_AGENTS;
        }
        sleep_ms(200);
    } else if (button_is_pressed(BTN_SELECT)) {
        if (current_state == STATE_LIST) current_state = STATE_DETAIL;
        else if (current_state == STATE_DETAIL) current_state = STATE_ACTION;
        else if (current_state == STATE_ACTION) {
            current_state = STATE_LIST;
        }
        sleep_ms(200);
    }
}
