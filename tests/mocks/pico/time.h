#ifndef MOCK_PICO_TIME_H
#define MOCK_PICO_TIME_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint64_t _private_us_since_boot;
} absolute_time_t;

extern uint64_t mock_absolute_time_us;

inline absolute_time_t get_absolute_time() {
    absolute_time_t t;
    t._private_us_since_boot = mock_absolute_time_us;
    return t;
}

inline int64_t absolute_time_diff_us(absolute_time_t from, absolute_time_t to) {
    return (int64_t)(to._private_us_since_boot - from._private_us_since_boot);
}

// Support for repeating_timer
struct repeating_timer {
    int64_t delay_ms;
    bool (*callback)(struct repeating_timer *t);
    void *user_data;
};

inline bool add_repeating_timer_ms(int64_t delay_ms, bool (*callback)(struct repeating_timer *t), void *user_data, struct repeating_timer *t) {
    t->delay_ms = delay_ms;
    t->callback = callback;
    t->user_data = user_data;
    return true;
}

inline bool cancel_repeating_timer(struct repeating_timer *t) {
    return true;
}

inline absolute_time_t delayed_by_us(absolute_time_t t, uint64_t us) {
    absolute_time_t new_t;
    new_t._private_us_since_boot = t._private_us_since_boot + us;
    return new_t;
}

#endif // MOCK_PICO_TIME_H
