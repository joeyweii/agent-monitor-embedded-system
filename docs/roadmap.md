# Project Roadmap: Agent Monitor

This roadmap outlines the development phases for the Agent Monitor system.

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
- [ ] Allocate second 40KB buffer for Ping-Pong strategy.
- [ ] Configure DMA Channel for SRAM-to-SPI transfer.
- [ ] Implement DMA Interrupt Service Routine (ISR) for buffer swapping.
- [ ] Benchmark refresh rates (Target: 30+ FPS).

## Phase 4: Information Layer
- [ ] Integrate 5x7 or 8x8 bitmap font table.
- [ ] Implement `draw_char()` and `draw_string()`.
- [ ] Add support for text alignment and color highlighting.
- [ ] Verify text clipping to prevent memory corruption.

## Phase 5: Communication Layer
- [ ] Implement Ring Buffer for Serial RX.
- [ ] Develop text-based parser for `SET:<agent_data>` commands.
- [ ] Implement Host-to-Device Heartbeat.
- [ ] Add Device-to-Host confirmation for button actions.

## Phase 6: Application Logic
- [ ] Implement Finite State Machine (FSM) for navigation.
- [ ] Design List View for agent carousel.
- [ ] Design Detail View for approval requests.
- [ ] Final visual polish (animations, status bar).
