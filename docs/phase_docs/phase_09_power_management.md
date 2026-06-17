# Phase 09 - Power Management

## Objective
Implement automatic screen dimming (sleep mode) after inactivity and wake-up functionality to optimize power consumption and prevent OLED burn-in.

## Key Achievements
- **PWM Backlight**: Configured RP2040 hardware PWM on `PIN_BL` for smooth, software-controlled backlight dimming instead of binary toggling.
- **Hardware Alarm Inactivity Timer**: Implemented a 30s timeout using `hardware_alarm` in the display module. This allows the system to enter a low-power state without polling.
- **Wake-on-Interrupt**: Updated button and protocol event handlers to instantly wake the UI and restore backlight brightness upon any user or serial activity.
- **Encapsulation**: Refactored power management logic into the display module to keep the application loop (`main.cpp`) clean and modular.

## Key Functions Implemented
```cpp
void display_init_backlight_timer(); // Sets up Hardware Alarm 0
void reset_backlight_alarm();        // Resets the inactivity timer on activity
void backlight_alarm_handler(uint); // ISR: Dims backlight on timeout
void display_set_backlight(uint16_t); // PWM control
void ui_wake_up();                   // Restores UI state and backlight
```
