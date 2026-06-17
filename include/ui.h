#ifndef UI_H
#define UI_H

#include "protocol.h"
#include "pico/time.h"

typedef enum {
    STATE_LIST,
    STATE_DETAIL
} UIState;

void ui_init();
void ui_update();
void ui_wake_up();

extern volatile bool ui_dirty_flag;
extern UIState current_state;
extern int selected_idx;
extern int scroll_offset;
extern absolute_time_t last_button_time;
extern bool is_asleep;

#endif // UI_H
