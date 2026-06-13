#include "ui.h"
#include "display.h"
#include "button.h"
#include <stdio.h>

static UIState current_state = STATE_LIST;
static int selected_idx = 0;

void ui_init() {
    current_state = STATE_LIST;
    selected_idx = 0;
}

void ui_update() {
    display_clear(COLOR_BLACK);
    display_draw_string(5, 5, "AGENT MONITOR", COLOR_YELLOW, COLOR_BLACK, 1);

    if (current_state == STATE_LIST) {
        for (int i = 0; i < MAX_AGENTS; i++) {
            AgentData* agent = protocol_get_agent(i);
            if (agent->is_active) {
                uint16_t color = (i == selected_idx) ? COLOR_WHITE : COLOR_GRAY;
                char buffer[64];
                snprintf(buffer, sizeof(buffer), "%c A%d: %s", (i == selected_idx) ? '>' : ' ', agent->id, agent->name);
                display_draw_string(5, 25 + (i * 15), buffer, color, COLOR_BLACK, 1);
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
