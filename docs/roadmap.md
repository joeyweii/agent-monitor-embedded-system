# Project Roadmap: Agent Monitor

This roadmap outlines the development phases for the Agent Monitor system.

## Milestone 1: Baseline System (Functional)

## Phase 1: Hardware Verification
- [x] Integrate Raspberry Pi Pico with 1.8" ST7735 LCD.
- [x] Establish basic SPI communication.
- [x] Configure 3-button navigation (PREV, NEXT, SELECT).
- [x] Calibrate RGB color mapping and screen offsets.
- [x] Implement Serial debugging over USB.

## Phase 2: Graphics Foundation
- [x] Allocate 40KB SRAM Framebuffer (RGB565).
- [x] Implement software drawing primitives (pixel, rect, clear).
- [x] Develop `display_flush()` for full-buffer SPI transmission.
- [x] Verify coordinate mapping accuracy.

## Phase 3: Performance Optimization
- [x] Allocate second 40KB buffer for Ping-Pong strategy.
- [x] Configure DMA Channel for SRAM-to-SPI transfer.
- [x] Implement asynchronous buffer swapping and flushing.
- [x] Benchmark refresh rates (Verified >60 FPS theoretical SPI throughput).

## Phase 4: Information Layer
- [x] Integrate 5x7 or 8x8 bitmap font table.
- [x] Implement `draw_char()` and `draw_string()`.
- [x] Add support for text alignment and color highlighting.
- [x] Verify text clipping to prevent memory corruption.

## Phase 5: Communication Layer
- [x] Implement Ring Buffer for Serial RX.
- [x] Develop length-prefixed parser for SET commands.
- [x] Implement Host-to-Device Heartbeat.
- [x] Add Device-to-Host confirmation for button actions.

## Phase 6: Application Logic
- [x] Implement Finite State Machine (FSM) for navigation.
- [x] Design List View for agent carousel.
- [x] Design Detail View for expanded agent status.
- [x] Implement interactive Action confirmation logic.

---

## Milestone 2: Professional UI & Performance

## Phase 7: Advanced UI Components
- [x] Implement "Inverted Header" style for the main title (Yellow bar with Black text).
- [x] Create graphical status icons (Done, Error, Waiting Input).
- [x] Implement dynamic "Running" animation (Pulse or Spinner widget).
- [x] Add List View layout optimization to show `<id>:<name> [ICON]`.
- [x] Implement text wrapping and scrolling in Detail View.

## Phase 8: Interrupt-Driven Architecture
- [x] Implement GPIO Interrupts for button handling to improve responsiveness.
- [x] Implement USB CDC RX Callback (TinyUSB) for asynchronous serial data.
- [x] Implement DMA Transfer-Complete Interrupt for non-blocking buffer swapping.
- [x] Transition main loop to `__wfi()` (Wait For Interrupt) for power efficiency.

## Phase 9: Partial Refresh (Dirty Rectangles)
- [ ] Implement logic to track "Dirty Rectangles" (only redraw/flush modified areas).
- [ ] Optimize SPI bandwidth for high-frequency UI animations.

## Phase 10: Power Management
- [ ] Implement software-controlled backlight dimming (PWM).
- [ ] Develop "Sleep Mode" logic (LCD off/CPU dormant) after inactivity timeout.
- [ ] Implement wake-on-interrupt for physical buttons.

---

## Future Improvements / V2
- [ ] **Hardware Acceleration**: Leverage RP2040 PIO or DMA-Memset for accelerated shape filling.
