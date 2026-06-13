/*
 * Protocol Specification:
 * Format: SET:<id>:<status_len>:<name_len>:<msg_len>:<payload>
 * 
 * Fields:
 * - SET: Command header
 * - id: 1 byte (0-3)
 * - status_len: Length of status string
 * - name_len: Length of name string
 * - msg_len: Length of message string
 * - payload: status + name + message (concatenated raw bytes)
 * 
 * Example: SET:0:7:7:13:RUNNINGAgent_AProcessing...
 */

#include "protocol.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static AgentData agents[MAX_AGENTS];
static uint8_t rx_buffer[512];
static int rx_idx = 0;

typedef enum {
    STATE_WAIT_FOR_CMD,
    STATE_READ_HEADER,
    STATE_READ_PAYLOAD
} ParserState;

static ParserState state = STATE_WAIT_FOR_CMD;
static uint8_t header[4]; // id, slen, nlen, mlen
static int header_idx = 0;
static int payload_total = 0;
static int payload_idx = 0;

void protocol_init() {
    memset(agents, 0, sizeof(agents));
    for (int i = 0; i < MAX_AGENTS; i++) agents[i].id = i;
}

void protocol_update() {
    while (true) {
        int c = getchar_timeout_us(100); 
        if (c == PICO_ERROR_TIMEOUT) break;

        if (state == STATE_WAIT_FOR_CMD) {
            rx_buffer[rx_idx++] = (uint8_t)c;
            if (rx_idx >= 4) {
                if (strncmp((char*)rx_buffer, "SET:", 4) == 0) {
                    state = STATE_READ_HEADER;
                    header_idx = 0;
                    rx_idx = 0;
                } else {
                    memmove(rx_buffer, rx_buffer + 1, rx_idx - 1);
                    rx_idx--;
                }
            }
        } else if (state == STATE_READ_HEADER) {
            rx_buffer[rx_idx++] = (uint8_t)c;
            if (c == ':') {
                rx_buffer[rx_idx - 1] = '\0';
                header[header_idx++] = atoi((char*)rx_buffer);
                rx_idx = 0;
                if (header_idx == 4) {
                    payload_total = header[1] + header[2] + header[3];
                    state = STATE_READ_PAYLOAD;
                    payload_idx = 0;
                    rx_idx = 0;
                }
            }
        } else if (state == STATE_READ_PAYLOAD) {
            rx_buffer[rx_idx++] = (uint8_t)c;
            payload_idx++; 
            if (payload_idx >= payload_total) {
                uint8_t id = header[0];
                if (id < MAX_AGENTS) {
                    memcpy(agents[id].status, rx_buffer, header[1]);
                    agents[id].status[header[1]] = '\0';
                    memcpy(agents[id].name, rx_buffer + header[1], header[2]);
                    agents[id].name[header[2]] = '\0';
                    memcpy(agents[id].message, rx_buffer + header[1] + header[2], header[3]);
                    agents[id].message[header[3]] = '\0';
                    agents[id].is_active = true;
                    agents[id].is_dirty = true;
                }
                state = STATE_WAIT_FOR_CMD;
                rx_idx = 0;
                header_idx = 0;
                memset(rx_buffer, 0, sizeof(rx_buffer));
            }
        }
    }
}

AgentData* protocol_get_agent(uint8_t id) {
    if (id < MAX_AGENTS) return &agents[id];
    return NULL;
}
