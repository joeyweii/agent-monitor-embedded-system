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
#include "ring_buffer.h"
#include "ui.h"
#include "display.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tusb.h"

extern int8_t selected_idx;
static AgentData agents[MAX_AGENTS];
static ring_buffer_t rx_ring;
volatile bool protocol_event_flag = false;

// TinyUSB RX Callback (Interrupt Context)
// This function is called automatically by the USB stack when data arrives.
void tud_cdc_rx_cb(uint8_t itf) {
    while (tud_cdc_available()) {
        uint8_t c = tud_cdc_read_char();
        if (ring_buffer_push(&rx_ring, c)) {
            protocol_event_flag = true;
        }
    }
}

typedef enum {
    STATE_WAIT_FOR_CMD,
    STATE_PARSE_CMD,
    STATE_READ_HEADER,
    STATE_READ_PAYLOAD,
    STATE_READ_DEL_ID
} ParserState;

static ParserState state = STATE_WAIT_FOR_CMD;
static char cmd_buffer[5];
static int cmd_idx = 0;
static uint8_t header[4]; // id, slen, nlen, mlen
static int header_idx = 0;
static int payload_total = 0;
static int payload_idx = 0;
static uint8_t payload_buffer[MAX_MSG_LEN + MAX_NAME_LEN + MAX_STATUS_LEN];
static uint8_t del_id = 0;

AgentStatus string_to_status(const char* status_str) {
    if (strcmp(status_str, "RUNNING") == 0) return AGENT_STATUS_RUNNING;
    if (strcmp(status_str, "INPUT") == 0) return AGENT_STATUS_INPUT;
    if (strcmp(status_str, "ERROR") == 0) return AGENT_STATUS_ERROR;
    return AGENT_STATUS_DONE;
}

void protocol_init() {
    memset(agents, 0, sizeof(agents));
    for (int i = 0; i < MAX_AGENTS; i++) {
        agents[i].id = i;
        agents[i].status = AGENT_STATUS_DONE;
    }
    ring_buffer_init(&rx_ring);
}

void handle_protocol_event() {
    ui_wake_up();
    reset_backlight_alarm();

    uint8_t c;
    while (ring_buffer_pop(&rx_ring, &c)) {
        switch (state) {
            case STATE_WAIT_FOR_CMD:
                if (c == 'S' || c == 'D') {
                    cmd_buffer[0] = c;
                    cmd_idx = 1;
                    state = STATE_PARSE_CMD;
                }
                break;

            case STATE_PARSE_CMD:
                cmd_buffer[cmd_idx++] = c;
                if (cmd_idx == 4) {
                    cmd_buffer[4] = '\0';
                    if (strcmp(cmd_buffer, "SET:") == 0) {
                        state = STATE_READ_HEADER;
                        header_idx = 0;
                    } else if (strcmp(cmd_buffer, "DEL:") == 0) {
                        state = STATE_READ_DEL_ID;
                    } else {
                        state = STATE_WAIT_FOR_CMD;
                    }
                }
                break;

            case STATE_READ_DEL_ID:
                del_id = c - '0';
                if (del_id < MAX_AGENTS) {
                    agents[del_id].is_active = false;
                    if (selected_idx == del_id) {
                        selected_idx = -1;
                        for (int i = 0; i < MAX_AGENTS; i++) {
                            if (agents[i].is_active) {
                                selected_idx = i;
                                break;
                            }
                        }
                    }
                }
                state = STATE_WAIT_FOR_CMD;
                break;

            case STATE_READ_HEADER: {
                static char val_buf[8];
                static int val_idx = 0;
                if (c == ':') {
                    val_buf[val_idx] = '\0';
                    header[header_idx++] = atoi(val_buf);
                    val_idx = 0;
                    if (header_idx == 4) {
                        payload_total = header[1] + header[2] + header[3];
                        state = STATE_READ_PAYLOAD;
                        payload_idx = 0;
                    }
                } else if (c >= '0' && c <= '9') {
                    if (val_idx < 7) val_buf[val_idx++] = c;
                } else {
                    // Invalid character in header, reset
                    state = STATE_WAIT_FOR_CMD;
                    val_idx = 0;
                }
                break;
            }

            case STATE_READ_PAYLOAD:
                payload_buffer[payload_idx++] = c;
                if (payload_idx >= payload_total) {
                    uint8_t id = header[0];
                    if (id < MAX_AGENTS) {
                        char status_tmp[MAX_STATUS_LEN];
                        int slen = (header[1] >= MAX_STATUS_LEN) ? MAX_STATUS_LEN - 1 : header[1];
                        memcpy(status_tmp, payload_buffer, slen);
                        status_tmp[slen] = '\0';
                        agents[id].status = string_to_status(status_tmp);

                        int nlen = (header[2] >= MAX_NAME_LEN) ? MAX_NAME_LEN - 1 : header[2];
                        memcpy(agents[id].name, payload_buffer + header[1], nlen);
                        agents[id].name[nlen] = '\0';

                        int mlen = (header[3] >= MAX_MSG_LEN) ? MAX_MSG_LEN - 1 : header[3];
                        memcpy(agents[id].message, payload_buffer + header[1] + header[2], mlen);
                        agents[id].message[mlen] = '\0';

                        agents[id].is_active = true;
                        if (selected_idx == -1) selected_idx = id;
                        agents[id].is_dirty = true;
                    }
                    state = STATE_WAIT_FOR_CMD;
                }
                break;
        }
    }
}

AgentData* protocol_get_agent(uint8_t id) {
    if (id < MAX_AGENTS) return &agents[id];
    return NULL;
}
