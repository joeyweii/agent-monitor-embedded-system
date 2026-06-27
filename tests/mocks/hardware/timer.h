#ifndef MOCK_HARDWARE_TIMER_H
#define MOCK_HARDWARE_TIMER_H

#include "pico/time.h"

// Hardware alarm mocks for display backlight timer if compiling display.cpp,
// though here we compile mock_display.cpp instead. We still provide these just in case.
inline void hardware_alarm_claim(uint alarm_num) {}
inline void hardware_alarm_set_callback(uint alarm_num, void (*callback)(uint alarm_num)) {}
inline void hardware_alarm_cancel(uint alarm_num) {}
inline void hardware_alarm_set_target(uint alarm_num, absolute_time_t target) {}

#endif // MOCK_HARDWARE_TIMER_H
