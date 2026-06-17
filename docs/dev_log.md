# Agent Monitor - Engineering Dev Log

## 2026-06-10 | Phase 01: Hardware Verification Complete
**Reference**: [Phase 01: Hardware Verification](phase_docs/phase_01_hardware_verification.md)

### Accomplishments
- **LCD Integration**: Verified SPI communication with ST7735. Solved Red-Blue swap by updating `MADCTL` to RGB mode.
- **Edge Artifacts**: Resolved random noise on screen edges by applying `X=+2, Y=+1` offsets in `set_window`.
- **Button Matrix**: Fixed 90-degree orientation error for tactile switches. Implemented 3-button (PREV, NEXT, SELECT) navigation logic.

---

## 2026-06-12 | Phase 02: Graphics Foundation Complete
**Reference**: [Phase 02: Graphics Foundation](phase_docs/phase_02_graphics_foundation.md)

### Accomplishments
- **Modular Refactoring**: Migrated from monolithic `main.cpp` to a structured `src/` and `include/` architecture.
- **Framebuffer Integration**: Implemented a 40KB SRAM-based framebuffer (RGB565).
- **Endianness Correction**: Solved the Little-Endian (RP2040) to Big-Endian (ST7735) color mapping issue via a row-by-row swap buffer.

---

## 2026-06-12 | Phase 03: Performance Optimization Complete
**Reference**: [Phase 03: Performance Optimization](phase_docs/phase_03_performance_optimization.md)

### Accomplishments
- **Pure 16-bit Data Path**: Optimized both SPI and DMA to operate in synchronized 16-bit mode.
- **Double Buffering**: Implemented a Ping-Pong buffer system (80KB total) for flicker-free animations.
- **Asynchronous DMA**: Decoupled CPU and display, utilizing DMA for high-speed transfers.
- **Event-Driven UI**: Implemented non-blocking input and "Dirty Flag" rendering.

---

## 2026-06-12 | Phase 04: Information Layer Complete
**Reference**: [Phase 04: Information Layer](phase_docs/phase_04_information_layer.md)

### Accomplishments
- **Bitmap Font Engine**: Integrated 5x7 font table and implemented `draw_char`/`draw_string` primitives.
- **Dynamic Scaling**: Added size multiplier support for UI elements.
- **Bounds Management**: Implemented pixel-level clipping for safe text rendering.

---

## 2026-06-12 | Phase 05: Communication Layer Complete
**Reference**: [Phase 05: Communication Layer](phase_docs/phase_05_communication_layer.md)

### Accomplishments
- **Length-Prefixed Protocol**: Implemented a robust serial command parser that handles arbitrary message content.
- **Asynchronous RX**: Developed a non-blocking Serial parser utilizing a 512-byte ring buffer.
- **UI Integration**: Connected dynamic agent data parsing with the rendering engine using a "Dirty Flag" trigger.

---

## 2026-06-13 | Phase 06: Application Logic Complete
**Spec Reference**: [Phase 06: Application Logic](phase_docs/phase_06_application_logic.md)

### Accomplishments
- **UI Finite State Machine**: Architected an event-driven FSM for screen navigation (List, Detail).
- **Interactive Navigation**: Implemented intuitive button mappings, including a "Back" function (`PREV` in Detail view).
- **System Integration**: Unified Communication, Graphics, and Font modules into a cohesive dashboard.

---

## 2026-06-13 | Phase 07: Advanced UI Components Complete
**Reference**: [Phase 07: Advanced UI Components](phase_docs/phase_07_advanced_ui_components.md)

### Accomplishments
- **Inverted Header**: Implemented visual hierarchy with a high-contrast yellow header bar.
- **Status Icon Engine**: Created graphical indicators (Checkmark, X, Question Mark) for agent statuses.
- **Spinner Animation**: Added a dynamic blue spinner for agents with `RUNNING` status.
- **Card UI**: Refined the Detail view with borders, separators, and accurate color mapping (e.g., Rose-colored names).
- **Scrolling**: Added text-wrapping and scroll support in Detail view.

---

## 2026-06-16 | Phase 08: Interrupt-Driven Architecture Complete
**Reference**: [Phase 08: Interrupt-Architecture](phase_docs/phase_08_interrupt_architecture.md)

### Accomplishments
- **GPIO Interrupts**: Replaced button polling with high-priority edge-triggered interrupts.
- **DMA Transfer IRQ**: Implemented a DMA completion handler to manage buffer swapping and Chip Select (CS) logic asynchronously.
- **USB CDC Callbacks**: Integrated TinyUSB `tud_cdc_rx_cb` for interrupt-driven serial data reception into a ring buffer.
- **Power Optimization**: Switched main loop to `__wfi()` (Wait For Interrupt), allowing the CPU to enter low-power sleep between events.
- **API Refactoring**: Modernized protocol and UI handling with structured enums (`AgentStatus`) and renamed event handlers (`handle_protocol_event`, `handle_button_event`).

---

## 2026-06-17 | Phase 09: Power Management Complete
**Reference**: [Phase 09: Power Management](phase_docs/phase_09_power_management.md)


### Accomplishments
- **PWM Backlight**: Configured hardware PWM for software-controlled backlight dimming.
- **Hardware Alarm Inactivity Timer**: Implemented a 30s timeout using `hardware_alarm` in `display.cpp` to dim the screen.
- **Wake-on-Interrupt**: Updated button and protocol event handlers to instantly wake the UI/backlight.
- **Encapsulation**: Refactored power management logic into the display module, keeping the main loop efficient and modular.
