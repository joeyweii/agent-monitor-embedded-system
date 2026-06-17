/**
 * @file font.h
 * @brief Bitmap font definitions for text rendering.
 */

#ifndef FONT_H
#define FONT_H

#include <stdint.h>

/**
 * @brief 5x7 Font bitmap table (ASCII 32-126).
 * Each character is represented by 5 bytes, where each byte is a column.
 */
extern const uint8_t font_5x7[95][5];

#endif // FONT_H
