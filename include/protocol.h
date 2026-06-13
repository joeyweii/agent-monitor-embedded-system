#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "pico/stdlib.h"

#define MAX_AGENTS 4
#define MAX_NAME_LEN 16
#define MAX_STATUS_LEN 16
#define MAX_MSG_LEN 64

typedef struct {
    uint8_t id;
    char name[MAX_NAME_LEN];
    char status[MAX_STATUS_LEN];
    char message[MAX_MSG_LEN];
    bool is_active;
    bool is_dirty; // UI needs to know when to redraw
} AgentData;

void protocol_init();
void protocol_update();
AgentData* protocol_get_agent(uint8_t id);

#endif // PROTOCOL_H
