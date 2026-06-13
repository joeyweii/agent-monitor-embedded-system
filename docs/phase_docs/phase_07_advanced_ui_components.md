# Phase Spec: 07 - Advanced UI Components

## Objective
Transform the dashboard from a text-heavy interface into a polished, glanceable UI by introducing high-contrast headers, graphical status icons, and dynamic animations.

## Technical Requirements

### 1. Inverted Header
- Draw a solid rectangle (`COLOR_YELLOW`) at the top of the screen (e.g., 0,0 to 128,15).
- Render "AGENT MONITOR" centered within the bar using `COLOR_BLACK` and `COLOR_YELLOW` as background.

### 2. Status Icon Engine
Implement primitive-based icons (approx. 10x10 pixels):
- **DONE**: A green 'V' (two thick lines).
- **ERROR**: A red triangle with a central exclamation point.
- **INPUT**: A yellow question mark (assigned to "Waiting for User Input").

### 3. Running Animation (The Spinner)
- A dynamic icon that changes every frame (or every few frames).
- Logic: Increment a `global_frame_counter`. Every update, draw a rotating square or a pulsing block to indicate active processing.

### 4. Optimized List Layout
- Format: `> A[ID]: [NAME] [ICON]`
- Align icons to the right edge of the screen for maximum glanceability.

## Success Criteria
1.  **Glanceability**: A user can identify the status of all 4 agents within 1 second.
2.  **Animation**: The "Running" indicator moves smoothly without stuttering (leveraging Phase 3's DMA).
3.  **Contrast**: The header clearly separates system information from agent data.
