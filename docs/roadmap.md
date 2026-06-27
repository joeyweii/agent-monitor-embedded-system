# Project Roadmap: Agent Monitor

This roadmap outlines the development phases for the Agent Monitor system.

## Milestone 1: Baseline System (Functional)

## Phase 1: Hardware Verification
- [x] Integrate Raspberry Pi Pico with 1.8" ST7735 LCD.
- [x] Establish basic SPI communication.
- [x] Configure 3-button navigation (PREV, NEXT, SELECT).
- [x] Calibrate RGB color mapping and screen offsets.
- [x] Implement Serial debugging over USB.

**Reference**: [Phase 01: Hardware Verification](phase_docs/phase_01_hardware_verification.md)

## Phase 2: Graphics Foundation
- [x] Allocate 40KB SRAM Framebuffer (RGB565).
- [x] Implement software drawing primitives (pixel, rect, clear).
- [x] Develop `display_flush()` for full-buffer SPI transmission.
- [x] Verify coordinate mapping accuracy.

**Reference**: [Phase 02: Graphics Foundation](phase_docs/phase_02_graphics_foundation.md)

## Phase 3: Performance Optimization
- [x] Allocate second 40KB buffer for Ping-Pong strategy.
- [x] Configure DMA Channel for SRAM-to-SPI transfer.
- [x] Implement asynchronous buffer swapping and flushing.
- [x] Benchmark refresh rates (Verified >60 FPS theoretical SPI throughput).

**Reference**: [Phase 03: Performance Optimization](phase_docs/phase_03_performance_optimization.md)

## Phase 4: Information Layer
- [x] Integrate 5x7 or 8x8 bitmap font table.
- [x] Implement `draw_char()` and `draw_string()`.
- [x] Add support for text alignment and color highlighting.
- [x] Verify text clipping to prevent memory corruption.

**Reference**: [Phase 04: Information Layer](phase_docs/phase_04_information_layer.md)

## Phase 5: Communication Layer
- [x] Implement Ring Buffer for Serial RX.
- [x] Develop length-prefixed parser for SET commands.
- [x] Implement Host-to-Device Heartbeat.
- [x] Add Device-to-Host confirmation for button actions.

**Reference**: [Phase 05: Communication Layer](phase_docs/phase_05_communication_layer.md)

## Phase 6: Application Logic
- [x] Implement Finite State Machine (FSM) for navigation.
- [x] Design List View for agent carousel.
- [x] Design Detail View for expanded agent status.
- [x] Implement interactive Action confirmation logic.

**Reference**: [Phase 06: Application Logic](phase_docs/phase_06_application_logic.md)

---

## Milestone 2: Professional UI & Performance

## Phase 7: Advanced UI Components
- [x] Implement "Inverted Header" style for the main title (Yellow bar with Black text).
- [x] Create graphical status icons (Done, Error, Waiting Input).
- [x] Implement dynamic "Running" animation (Pulse or Spinner widget).
- [x] Add List View layout optimization to show `<id>:<name> [ICON]`.
- [x] Implement text wrapping and scrolling in Detail View.

**Reference**: [Phase 07: Advanced UI Components](phase_docs/phase_07_advanced_ui_components.md)

## Phase 8: Interrupt-Driven Architecture
- [x] Implement GPIO Interrupts for button handling to improve responsiveness.
- [x] Implement USB CDC RX Callback (TinyUSB) for asynchronous serial data.
- [x] Implement DMA Transfer-Complete Interrupt for non-blocking buffer swapping.
- [x] Transition main loop to `__wfi()` (Wait For Interrupt) for power efficiency.

**Reference**: [Phase 08: Interrupt-Architecture](phase_docs/phase_08_interrupt_architecture.md)

## Phase 9: Power Management
- [x] Implement software-controlled backlight dimming (PWM).
- [x] Develop "Sleep Mode" logic (LCD off/CPU dormant) after inactivity timeout.
- [x] Implement wake-on-interrupt for physical buttons.

**Reference**: [Phase 09: Power Management](phase_docs/phase_09_power_management.md)

---

## Future Improvements / V2
- [ ] **Hardware Acceleration**: Leverage RP2040 PIO or DMA-Memset for accelerated shape filling.
