# Phase Spec: 03 - Performance Optimization

## Objective
Enable Direct Memory Access (DMA) and Double Buffering (Ping-Pong) to achieve high-frame-rate, asynchronous display updates. This frees the CPU to handle serial communication and application logic while the screen flushes.

## Technical Requirements
- **Memory Allocation**: Allocate a second 40KB buffer (`uint16_t framebuffer_b[128 * 160]`).
- **DMA Configuration**:
    - Configure one DMA channel for SRAM-to-SPI transmission.
    - Set transfer size to 16-bit.
    - Use SPI TX DREQ (Data Request) to pace the transfer.
- **Asynchronous Flow**:
    - `display_flush()` triggers DMA and returns immediately.
    - Implement a `display_is_busy()` check using `dma_channel_is_busy()`.
- **Endianness Strategy**: Optimization of the row-by-row swap.

## Key Functions to Implement
```cpp
void display_init_dma();
void display_swap_buffers();
bool display_is_busy();
void display_flush_async(); 
```

## Success Criteria
1.  CPU utilization drops significantly during screen flushes.
2.  Refresh rate exceeds 30 FPS.
3.  Zero "tearing" artifacts observed during fast animations.
