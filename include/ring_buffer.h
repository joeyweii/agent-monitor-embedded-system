/**
 * @file ring_buffer.h
 * @brief Thread-safe circular buffer for serial data reception.
 */

#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/** @brief Size of the serial reception buffer in bytes. */
#define RING_BUFFER_SIZE 512

/**
 * @brief Ring buffer control structure.
 */
typedef struct {
    uint8_t buffer[RING_BUFFER_SIZE]; /**< Raw data storage. */
    volatile int head;                /**< Index for the next write operation. */
    volatile int tail;                /**< Index for the next read operation. */
} ring_buffer_t;

/**
 * @brief Initializes the ring buffer structure.
 * @param rb Pointer to the ring_buffer_t instance.
 */
void ring_buffer_init(ring_buffer_t *rb);

/**
 * @brief Pushes a byte into the buffer.
 * @param rb Pointer to the ring_buffer_t instance.
 * @param data The byte to store.
 * @return true if successful, false if the buffer is full.
 */
bool ring_buffer_push(ring_buffer_t *rb, uint8_t data);

/**
 * @brief Pops a byte from the buffer.
 * @param rb Pointer to the ring_buffer_t instance.
 * @param data Pointer to store the retrieved byte.
 * @return true if data was retrieved, false if the buffer is empty.
 */
bool ring_buffer_pop(ring_buffer_t *rb, uint8_t *data);

/**
 * @brief Checks if the buffer has any pending data.
 * @param rb Pointer to the ring_buffer_t instance.
 * @return true if empty, false otherwise.
 */
bool ring_buffer_is_empty(ring_buffer_t *rb);

#ifdef __cplusplus
}
#endif

#endif // RING_BUFFER_H
