# Phase 01 - Hardware Verification

## Objective
To ensure proper functionality of the hardware interfaces and underlying drivers.

## Key Achievements
*   **Successful ST7735 LCD Integration**: Established SPI communication protocol supporting RGB565 16-bit color rendering on a 1.8" TFT display.
*   **3-Button Navigation System**: Fully integrated PREV (GP26), NEXT (GP27), and SELECT (GP28) physical inputs with a circular navigation state machine.
*   **Stability & Monitoring**: Implemented a Heartbeat LED (GP25) and USB Serial debugging output to ensure persistent firmware operation and real-time state monitoring.

## Resolved Technical Issues
*   **Button Orientation Fix**: Resolved a critical short-circuit issue where B3F-1020 tactile switches were permanently closed due to a 90-degree orientation error on the breadboard.
*   **Color Calibration**:
    *   **Red-Blue Swap**: Fixed the "Yellow appears as Light Blue" issue by correcting the `MADCTL` (0x36) register, toggling the color space from BGR to **RGB**.
    *   **Inversion Correction**: Verified and disabled the `INVON` command to restore accurate color contrast.
*   **Edge Artifact Removal**: Addressed the misalignment between the 128x160 physical panel and the 132x162 driver chip RAM by implementing a hardware offset of **X=+2, Y=+1** in the `set_window` function, eliminating random noise lines on the edges.
