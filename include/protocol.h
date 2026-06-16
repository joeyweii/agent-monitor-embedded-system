#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "pico/stdlib.h"

#define MAX_AGENTS 5
#define MAX_NAME_LEN 16
#define MAX_STATUS_LEN 16
#define MAX_MSG_LEN 64

typedef enum {
    AGENT_STATUS_DONE,
    AGENT_STATUS_RUNNING,
    AGENT_STATUS_INPUT,
    AGENT_STATUS_ERROR
} AgentStatus;

typedef struct {
    uint8_t id;
    char name[MAX_NAME_LEN];
    AgentStatus status;
    char message[MAX_MSG_LEN];
    bool is_active;
    bool is_dirty; // UI needs to know when to redraw
} AgentData;

void protocol_init();
void handle_protocol_event();
AgentData* protocol_get_agent(uint8_t id);

extern volatile bool protocol_event_flag;

#endif // PROTOCOL_H
