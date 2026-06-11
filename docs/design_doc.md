# Design Doc: Agent Monitor Embedded System

**Author:** Gemini CLI / cywei  
**Status:** Draft  
**Last Updated:** 2026-06-10  

---

## 1. Summary
The **Agent Monitor** is a dedicated hardware peripheral designed to provide real-time status visibility for AI agents. Built on the Raspberry Pi Pico (RP2040) platform, it features a 1.8" TFT LCD and a 3-button navigation interface. The system provides a "smartwatch-like" glanceable experience, allowing users to monitor agent health, tasks, errors, and approval requests at any time—even when away from their workstation (e.g., during lunch)—without requiring active interaction with a PC.

## 2. Goals

### Goals
*   **Real-time Visibility**: Display status for at least 3 concurrent agents.
*   **Low Latency**: Provide a responsive UI with >30 FPS refresh rates using DMA SPI.
*   **Physical Interaction**: Allow users to approve or reject agent actions via hardware buttons.
*   **Robustness**: Handle serial communication errors and hardware noise gracefully.
*   **Developer Experience**: Maintain professional documentation and a clean C++17 codebase.

## 3. Proposed Design

### 3.1 System Architecture
The system follows a classic **Host-Peripheral** model:
1.  **Host (PC/Mac)**: A background service monitors agent states and pushes updates via USB Serial.
2.  **Peripheral (Pico)**: Receives serial packets, updates the internal state machine, and renders the UI to the LCD.

### 3.2 Hardware Design
*   **Microcontroller**: Raspberry Pi Pico (RP2040).
*   **Display**: ST7735 1.8" TFT LCD (128x160 resolution).
*   **Input**: 3x B3F-1020 Tactile Switches (PREV, NEXT, SELECT).

### 3.3 Software Architecture
Will update.

## 4. Alternatives Considered
Will update.

## 5. Cross-cutting Concerns
Will update.

## 6. Testing Plan
Will update.
