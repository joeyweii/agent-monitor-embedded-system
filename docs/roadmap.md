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
- [ ] Implement Finite State Machine (FSM) for navigation.
- [ ] Design List View for agent carousel.
- [ ] Design Detail View for approval requests.
- [ ] Final visual polish (animations, status bar).

## Future Improvements / V2
- [ ] Implement formal DMA Interrupt Service Routine (ISR) to further optimize CPU usage.
- [ ] Implement deeper sleep states for low-power operation.
