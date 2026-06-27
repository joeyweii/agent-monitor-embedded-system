#ifndef MOCK_PICO_STDLIB_H
#define MOCK_PICO_STDLIB_H

#include <stdint.h>
#include <stdbool.h>

typedef unsigned int uint;

inline void stdio_init_all() {}
inline void sleep_ms(uint32_t ms) {}

#endif // MOCK_PICO_STDLIB_H
