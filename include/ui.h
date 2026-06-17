/**
 * @file ui.h
 * @brief High-level UI state machine and layout orchestration.
 */

#ifndef UI_H
#define UI_H

#include "protocol.h"
#include "pico/time.h"
#include "ui_config.h"

/**
 * @brief UI Application States.
 */
typedef enum {
    STATE_LIST,   /**< Main agent list view. */
    STATE_DETAIL  /**< Detailed agent status view. */
} UIState;

/**
 * @brief Initializes the UI state machine and configuration.
 */
void ui_init();

/**
 * @brief Main UI update loop. Processes layout and rendering.
 */
void ui_update();

/**
 * @brief Wakes up the display/backlight if it was dimmed.
 */
void ui_wake_up();

/** @brief Flag indicating the UI needs a redraw. */
extern volatile bool ui_dirty_flag;

/** @brief Current active UI state. */
extern UIState current_state;

/** @brief Index of the currently selected agent (-1 if none). */
extern int8_t selected_idx;

/** @brief Vertical scroll offset for detail view. */
extern int scroll_offset;

/** @brief Timestamp of the last button interaction. */
extern absolute_time_t last_button_time;

/** @brief Global sleep status of the monitor. */
extern bool is_asleep;

#endif // UI_H
