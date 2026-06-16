# Phase 08 - Interrupt-Architecture

## Objective
Migrate the system from polling-based data handling to a fully interrupt-driven framework to improve responsiveness, reliability, and power efficiency.

## Key Achievements
- **GPIO Interrupts**: Replaced manual button polling with high-priority edge-triggered interrupts, ensuring zero-latency input handling.
- **DMA Transfer IRQ**: Implemented a DMA completion handler to manage display buffer swapping and SPI Chip Select (CS) logic asynchronously.
- **USB CDC Callbacks**: Integrated TinyUSB `tud_cdc_rx_cb` for interrupt-driven serial data reception, preventing packet loss during heavy CPU loads.
- **Power Optimization**: Switched the main loop to a `__wfi()` (Wait For Interrupt) model, allowing the RP2040 to enter a low-power sleep state between events.
- **Thread-Safe Data Path**: Developed a 512-byte ring buffer to safely transport data from interrupt contexts to the application logic.

## Key Functions Implemented
```cpp
void dma_handler();            // DMA IRQ 0: Signals display transfer completion
void gpio_irq_handler();       // GPIO IRQ: Captures button presses asynchronously
void tud_cdc_rx_cb(uint8_t);   // TinyUSB Callback: Triggered on serial data arrival
void handle_protocol_event();  // Processes serial data from ring buffer
void handle_button_event();    // Processes button state changes
```
