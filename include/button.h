#ifndef BUTTON_H
#define BUTTON_H

#include "pico/stdlib.h"

// Button Pins
#define BTN_PREV   26  // GP26 = Pin 31
#define BTN_NEXT   27  // GP27 = Pin 32
#define BTN_SELECT 28  // GP28 = Pin 34

// Onboard LED
#define LED_PIN   25

// API Functions
void buttons_init();
bool button_is_pressed(uint pin);
void led_set(bool state);
void led_toggle();

#endif // BUTTON_H
