/**
 * @file protocol.h
 * @brief Communication protocol and agent data management.
 */

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "pico/stdlib.h"

/** @brief Maximum number of concurrent agents supported. */
#define MAX_AGENTS 7
/** @brief Maximum length of an agent's name string. */
#define MAX_NAME_LEN 16
/** @brief Maximum length of an agent's status string. */
#define MAX_STATUS_LEN 16
/** @brief Maximum length of an agent's message string. */
#define MAX_MSG_LEN 64

/**
 * @brief Possible agent statuses.
 */
typedef enum {
    AGENT_STATUS_DONE,    /**< Agent has successfully finished its task. */
    AGENT_STATUS_RUNNING, /**< Agent is currently active. */
    AGENT_STATUS_INPUT,   /**< Agent is waiting for user input. */
    AGENT_STATUS_ERROR    /**< Agent encountered a failure. */
} AgentStatus;

/**
 * @brief Data structure representing an AI agent's state.
 */
typedef struct {
    uint8_t id;                /**< Unique ID for the agent (0 to MAX_AGENTS-1). */
    char name[MAX_NAME_LEN];   /**< Friendly name of the agent. */
    AgentStatus status;        /**< Current operational status. */
    char message[MAX_MSG_LEN]; /**< Detailed status or task message. */
    bool is_active;            /**< Whether this agent slot is currently in use. */
    bool is_dirty;             /**< Flag for UI to indicate data has changed. */
} AgentData;

/**
 * @brief Initializes the protocol manager and clear all agent slots.
 */
void protocol_init();

/**
 * @brief Main protocol processing task.
 * Parses incoming serial data from the ring buffer into AgentData structures.
 */
void handle_protocol_event();

/**
 * @brief Retrieves a pointer to an agent's data.
 * @param id The agent ID (0 to MAX_AGENTS-1).
 * @return Pointer to AgentData, or NULL if ID is invalid.
 */
AgentData* protocol_get_agent(uint8_t id);

/** @brief Flag indicating new serial data is available to be parsed. */
extern volatile bool protocol_event_flag;

#endif // PROTOCOL_H
