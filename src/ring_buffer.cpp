#include "ring_buffer.h"

void ring_buffer_init(ring_buffer_t *rb) {
    rb->head = 0;
    rb->tail = 0;
}

bool ring_buffer_push(ring_buffer_t *rb, uint8_t data) {
    int next = (rb->head + 1) % RING_BUFFER_SIZE;
    if (next == rb->tail) return false; // Full
    rb->buffer[rb->head] = data;
    rb->head = next;
    return true;
}

bool ring_buffer_pop(ring_buffer_t *rb, uint8_t *data) {
    if (rb->head == rb->tail) return false; // Empty
    *data = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) % RING_BUFFER_SIZE;
    return true;
}

bool ring_buffer_is_empty(ring_buffer_t *rb) {
    return rb->head == rb->tail;
}
