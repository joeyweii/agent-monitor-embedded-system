#include "test_framework.h"
#include "ring_buffer.h"

void test_ring_buffer_init() {
    ring_buffer_t rb;
    ring_buffer_init(&rb);
    ASSERT_TRUE(ring_buffer_is_empty(&rb));
    
    uint8_t data = 0;
    ASSERT_FALSE(ring_buffer_pop(&rb, &data));
}

void test_ring_buffer_push_pop() {
    ring_buffer_t rb;
    ring_buffer_init(&rb);
    
    ASSERT_TRUE(ring_buffer_push(&rb, 42));
    ASSERT_FALSE(ring_buffer_is_empty(&rb));
    
    uint8_t data = 0;
    ASSERT_TRUE(ring_buffer_pop(&rb, &data));
    ASSERT_EQ(data, 42);
    ASSERT_TRUE(ring_buffer_is_empty(&rb));
}

void test_ring_buffer_fifo_order() {
    ring_buffer_t rb;
    ring_buffer_init(&rb);
    
    for (int i = 0; i < 10; i++) {
        ASSERT_TRUE(ring_buffer_push(&rb, i));
    }
    
    for (int i = 0; i < 10; i++) {
        uint8_t data = 0;
        ASSERT_TRUE(ring_buffer_pop(&rb, &data));
        ASSERT_EQ(data, i);
    }
    ASSERT_TRUE(ring_buffer_is_empty(&rb));
}

void test_ring_buffer_full_condition() {
    ring_buffer_t rb;
    ring_buffer_init(&rb);
    
    // Max capacity is RING_BUFFER_SIZE - 1 = 511
    for (int i = 0; i < RING_BUFFER_SIZE - 1; i++) {
        ASSERT_TRUE(ring_buffer_push(&rb, i % 256));
    }
    
    // 512th push should fail
    ASSERT_FALSE(ring_buffer_push(&rb, 99));
    
    // Pop one, capacity should free up by 1
    uint8_t popped_val = 0;
    ASSERT_TRUE(ring_buffer_pop(&rb, &popped_val));
    ASSERT_EQ(popped_val, 0);
    
    // Now push should succeed
    ASSERT_TRUE(ring_buffer_push(&rb, 99));
    
    // Check remaining items
    for (int i = 1; i < RING_BUFFER_SIZE - 1; i++) {
        uint8_t val = 0;
        ASSERT_TRUE(ring_buffer_pop(&rb, &val));
        ASSERT_EQ(val, i % 256);
    }
    
    // The last item should be the one we pushed (99)
    uint8_t last_val = 0;
    ASSERT_TRUE(ring_buffer_pop(&rb, &last_val));
    ASSERT_EQ(last_val, 99);
    
    ASSERT_TRUE(ring_buffer_is_empty(&rb));
}

void register_ring_buffer_tests(std::vector<TestCase>& tests) {
    tests.push_back({"ring_buffer_init", test_ring_buffer_init});
    tests.push_back({"ring_buffer_push_pop", test_ring_buffer_push_pop});
    tests.push_back({"ring_buffer_fifo_order", test_ring_buffer_fifo_order});
    tests.push_back({"ring_buffer_full_condition", test_ring_buffer_full_condition});
}
