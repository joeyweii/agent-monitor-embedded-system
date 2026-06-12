# Phase Spec: 02 - Graphics Foundation

## Objective
Transition from immediate SPI rendering to a memory-backed graphics architecture. This allows for complex UI composition (layering, text, shapes) without flickering.

## Technical Requirements
- **Memory Allocation**: `uint16_t framebuffer[128 * 160]` (Exactly 40,960 bytes).
- **Coordinate System**: (0,0) at top-left. Correct for the `X=+2, Y=+1` hardware offset.
- **Rendering Loop**: Clear buffer -> Draw components -> Flush via SPI.

## Key Functions to Implement
```cpp
void clear_buffer(uint16_t color);
void draw_pixel(int x, int y, uint16_t color);
void draw_rect(int x, int y, int w, int h, uint16_t color);
void display_flush(); // Full buffer transmission
```

## Success Criteria
1.  Full-screen updates complete without visible "scanning" lines.
2.  Complex shapes (e.g., overlapping rectangles) render correctly in memory before appearing on screen.
3.  FPS measured via debug pin or Serial.
