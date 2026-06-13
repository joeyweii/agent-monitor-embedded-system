# Design Doc: Agent Monitor Embedded System

**Author:** Gemini CLI / joeyweii  
**Status:** In-Review  
**Last Updated:** 2026-06-12

---

## 1. Abstract / Summary
The **Agent Monitor** is a dedicated hardware peripheral designed to provide real-time status visibility for autonomous AI agents. Built on the Raspberry Pi Pico (RP2040) platform, it features a 1.8" TFT LCD and a 3-button navigation interface. The system provides a "smartwatch-like" glanceable experience, allowing users to monitor agent status, health, tasks, and approval requests at any time—even when away from their workstation (e.g., during lunch)—without requiring active interaction with a PC.

## 2. System Architecture

### 2.1 Overview Diagram
```mermaid
graph TD
    subgraph Host_PC
        AgentRunner[AI Agent Runner] --> |JSON Status| HostService[Python Monitor Service]
        HostService --> |Serial Packets| SerialUSB[USB CDC]
    end

    subgraph Pico_Firmware
        SerialUSB --> |UART RX| ProtocolParser[Serial Protocol Parser]
        ProtocolParser --> |Update State| StateMachine[UI State Machine]
        
        StateMachine --> |Draw Commands| GFXLib[Graphics Library]
        GFXLib --> |Write Pixels| FB_Manager[Double Buffer Manager]
        
        FB_Manager --> |Swap| DMA[DMA Controller]
        DMA --> |SPI TX| LCD_Driver[ST7735 Driver]
    end

    LCD_Driver --> |Visuals| LCD_Glass[1.8 Inch TFT]
    Buttons[3x Tactile Buttons] --> |GPIO IRQ| StateMachine
```

### 2.2 Hardware Design
For detailes, refer to the **[Breadboard Layout & Wiring Guide](breadboard_layout.md)**.

### 2.3 Double Buffering (Ping-Pong)
To achieve flicker-free animations and high-performance rendering, the system employs a **Double Buffering** strategy:
*   **Back Buffer**: The CPU performs all drawing operations (clearing, rectangles, text) into this 40KB SRAM array.
*   **Front Buffer**: Currently being read by the DMA controller and transmitted to the LCD.
*   **Synchronization**: Upon completion of a frame, the pointers are swapped. The system uses a DMA Interrupt to track transfer completion.

### 2.4 Asynchronous DMA SPI
The CPU does not block during screen updates.
*   **Transfer Size**: 128 * 160 * 2 = 40,960 bytes per frame.
*   **SPI Speed**: 24MHz (target).
*   **DMA Configuration**: 16-bit transfers from SRAM to SPI TX FIFO.

### 2.5 Communication Protocol
A robust, length-prefixed binary/text protocol over USB Serial:
- `SET:<id>:<status_len>:<name_len>:<msg_len>:<payload>`
- **Example**: `SET:1:7:7:24:RUNNING:Agent_A:Processing user query...`
- **Benefit**: Immune to delimiter collisions; allows multi-sentence messages; memory-safe.
- **Parsing**: Two-stage state machine that consumes header metadata, then exactly N payload bytes.

### 2.6 UI State Machine
The system operates as a finite state machine (FSM):
*   **IDLE**: Low-power/Dimmed state.
*   **LIST_VIEW**: Main carousel of active agents.
*   **DETAIL_VIEW**: Expanded view with action buttons (Approve/Reject).
*   **ERROR_VIEW**: System-wide alert for connection loss.

### 2.7 Power Management
As a portable-ready peripheral, power efficiency is a core design pillar:
*   **Dynamic Backlight**: PWM-based dimming for idle states.
*   **Deep Sleep**: Utilization of RP2040 `dormant` mode during extended inactivity.
*   **Event-based Wake**: Hardware interrupts on button pins to wake the processor.

## 3. Development Roadmap
For detailes, refer to the **[Roadmap](roadmap.md)**.
The project is divided into logical milestones:

### Milestone 1: Baseline System (Phases 1-6)
Core infrastructure including driver, graphics foundation, communication protocol, and basic UI navigation.

### Milestone 2: Professional UI & Performance (Phases 7-10)
Advanced graphical widgets, interrupt-driven core, partial screen rendering, and power management.

## 4. Project Goals
*   **Performance**: Stable 30+ FPS UI.
*   **Visibility**: Real-time status for up to 4 concurrent agents.
*   **Reliability**: Robust handling of serial noise and common-ground stability.
*   **UX**: Intuitive 3-button navigation.
