#ifndef UI_H
#define UI_H

#include "protocol.h"

typedef enum {
    STATE_LIST,
    STATE_DETAIL,
    STATE_ACTION
} UIState;

void ui_init();
void ui_update();
void ui_handle_input();

#endif // UI_H
