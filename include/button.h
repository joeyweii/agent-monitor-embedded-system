/**
 * @file button.h
 * @brief GPIO button handling and input event management.
 */

#ifndef BUTTON_H
#define BUTTON_H

#include "pico/stdlib.h"

/** @brief Pin for the PREV button. */
#define BTN_PREV   26
/** @brief Pin for the NEXT button. */
#define BTN_NEXT   27
/** @brief Pin for the SELECT button. */
#define BTN_SELECT 28

/** @brief Pin for the onboard LED. */
#define LED_PIN   25

/**
 * @brief Initializes GPIO pins for buttons and onboard LED.
 * Sets up internal pull-ups and edge-triggered interrupts.
 */
void buttons_init();

/**
 * @brief Checks the current physical state of a button.
 * @param pin The GPIO pin to check.
 * @return true if pressed, false otherwise.
 */
bool button_is_pressed(uint pin);

/**
 * @brief Processes a button press event.
 * Handles debouncing and updates UI state machine.
 * @param gpio The GPIO pin that triggered the event.
 */
void handle_button_event(uint gpio);

/**
 * @brief Sets the state of the onboard LED.
 * @param state true for ON, false for OFF.
 */
void led_set(bool state);

/**
 * @brief Toggles the state of the onboard LED.
 */
void led_toggle();

/** @brief Flag indicating a button interrupt has occurred. */
extern volatile bool button_event_flag;

/** @brief The GPIO pin that triggered the current button event. */
extern volatile uint target_button_gpio;

#endif // BUTTON_H
