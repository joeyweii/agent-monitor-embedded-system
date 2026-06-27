# Design Decisions & Trade-offs

**Purpose:** Technical deep-dive — why each architectural choice was made, what was traded off, and what the real-world numbers look like.

---

## Summary Table

| # | Topic | Decision | Key Reason |
|---|---|---|---|
| 1 | Framebuffer Strategy | Double buffer (ping-pong) | Tear-free rendering; decouples CPU render from SPI transfer |
| 2 | SPI Transfer | Async DMA | 100% CPU availability for events & low-power sleep during transmission |
| 3 | Event Handling | Interrupt + `__wfi()` | Minimal power (drop core current from ~20mA to ~1.2mA); <1µs latency |
| 4 | Frame Rate | Dynamic FPS Adaptation | 0 FPS when static (WFI sleep); dynamic 10 FPS repeating timer when animating |
| 5 | Power Management | Hardware alarm + backlight PWM | Backlight dims after 10s inactivity; wakes on event; sleeps without CPU intervention |
| 6 | Protocol | Length-prefixed `SET`/`DEL` | Noise resilient; memory-safe (0 byte dynamic heap allocation); no escaping needed |
| 7 | Serial RX Buffer | Lock-free SPSC ring buffer | Safe concurrency between USB interrupt and main loop without disabling IRQs |

---

## 1. Direct Draw vs. Single Buffer vs. Double Buffer

### The Problem
The ST7735 LCD is 128×160 pixels, using 16-bit RGB565 color. A naive implementation draws directly to the display over SPI — meaning each primitive (rect, char, pixel) produces a separate SPI transaction, causing severe **visual tearing** as the display shows a partially-drawn frame mid-update.

### Options Considered

| Approach | Description | Tearing? | RAM Cost |
|---|---|---|---|
| **Direct draw** | Every primitive calls SPI write immediately | ✅ Yes | **0 KB** (no buffer) |
| **Single framebuffer** | Draw into SRAM, then flush the complete buffer to SPI | ❌ No | **40 KB** |
| **Double framebuffer** ✅ **(chosen)** | Back buffer for drawing, front buffer for transmission | ❌ No | **80 KB** |

### Numbers

- **Frame size:** 128 × 160 × 2 bytes = **40,960 bytes ≈ 40 KB** per buffer
- **Total framebuffer cost:** 2 × 40 KB = **80 KB**
- **RP2040 total SRAM:** 264 KB
- **Remaining SRAM after buffers:** 264 − 80 = **184 KB** for stack, heap, protocol state, font table (~475 B), ring buffer (512 B)

### Why Double Buffer Wins
While a single framebuffer stops tearing *during the draw stage*, the CPU is still blocked from modifying any pixels while the display interface is reading the buffer. If the drawing process takes time or is asynchronous, visual artifacts or pipeline stalls occur. 

Double buffering decouples the rendering context (`back_buffer`) from the active transmission context (`front_buffer`). The CPU writes to one while the other is read out.

### Trade-off Accepted
80 KB of SRAM is committed permanently to pixel storage. On a device with 264 KB total, this is 30% of all RAM. This is acceptable because the remaining 184 KB comfortably covers all other runtime needs, but would be untenable on smaller MCUs (e.g., ATmega328 with only 2 KB SRAM).

---

## 2. DMA vs. CPU-Driven SPI Transfer

> This section is the direct follow-on to §1. The double buffer resolves *tearing* — DMA resolves *CPU blocking*. Together they achieve both tear-free rendering **and** full CPU parallelism.

### The Problem
Sending 40,960 bytes over SPI requires feeding the SPI TX FIFO continuously. Without DMA, the CPU must execute a tight loop that stalls on the FIFO for each 16-bit word — blocking all other processing for the entire duration of the transfer.

### CPU Availability Analysis

Assume CPU render time for a full frame (clear + header + 7 agent rows + icons) ≈ **2 ms** at 125 MHz.
The physical transfer of 40,960 bytes at 24 MHz SPI takes **13.65 ms**.

**Scenario A — CPU blocking flush**
The CPU must wait for the SPI peripheral to finish transmitting every single word.
```
[──render 2ms──][────CPU blocks, SPI flush 13.65ms────]
▲                                                     ▲
└────────────────── Total 15.65 ms ───────────────────┘
```
- **CPU Idle/Blocked:** 13.65 ms (87.2% of the cycle)
- **CPU Available for events (Serial RX, Buttons):** ❌ 12.8% (Only during render)
- **Impact:** Significant risk of missing button edge interrupts or overflowing the USB RX buffer during a screen update.

**Scenario B — Async DMA** ✅ **(chosen)**
The CPU sets up the DMA transaction and immediately returns to process events or enter low-power sleep via `__wfi()`.
```
[──render 2ms──][kick DMA]
               [──────DMA streams frame 13.65ms (background)──────]
▲              ▲
│              └─ CPU is free / sleeping for 13.65ms (87.2% of the cycle)
└─ Total 2ms CPU active
```
- **CPU Idle/Blocked:** 0 ms (0% of the cycle)
- **CPU Available for events (Serial RX, Buttons):** ✅ 100% (The main loop can immediately respond to interrupts)
- **Impact:** Zero latency for input processing. The CPU can immediately enter sleep while the display updates in the background.

| Method | CPU Busy (Blocking) | CPU Available (Event Processing / Sleep) | Risk of Input Event Loss |
|---|---|---|---|
| Blocking flush | 13.65 ms (87.2%) | 2.0 ms (12.8%) | High |
| **Async DMA** | **0.0 ms (0.0%)** | **15.65 ms (100.0%)** | **None** |

### DMA Configuration (from `display.cpp`)
```cpp
channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_16);  // 16-bit per transfer (1 pixel)
channel_config_set_dreq(&dma_config, DREQ_SPI0_TX);               // Paced by SPI TX FIFO
channel_config_set_read_increment(&dma_config, true);              // Walk through framebuffer
channel_config_set_write_increment(&dma_config, false);            // Always write to SPI TX FIFO register
```

The DMA fires `DMA_IRQ_0` on completion. The handler clears the flag and releases CS, allowing the next `display_flush_async()` call to proceed.

### Non-Blocking Flush Guard (Frame Drop Policy)
```cpp
void display_flush_async() {
    if (display_is_busy()) return; // Drop frame if previous DMA not done
    ...
}
```
If the CPU renders a new frame before the previous DMA finishes, the new frame is **dropped** rather than queued. This is intentional: for a monitoring display where data changes at human-observable speed (seconds), a short-lived dropped frame is invisible and far preferable to a stale frame queue building up latency.

---

## 3. Polling vs. Interrupt-Driven Architecture

### The Problem
A polling loop (`while(true) { check_serial(); check_buttons(); draw(); }`) keeps the processor running at 100% duty cycle, burning unnecessary power checking registers for events that occur sporadically (e.g., occasional button presses or host updates).

### Options Considered

| Approach | Latency | Power / CPU Duty Cycle | Complexity |
|---|---|---|---|
| **Pure polling** | Variable (loop latency) | High (100% active duty cycle) | Low |
| **RTOS (FreeRTOS)** | Very Low | Low (automatic idle task hook) | High (Over-engineered) |
| **Interrupt + `__wfi()`** ✅ **(chosen)** | Very Low (immediate hardware wake) | Very Low (< 1% active when idle) | Low-Medium |

### Data & Power Analysis (Polling vs. Sleep)

- **Power State in Polling:** The RP2040 runs its twin Cortex-M0+ cores at 125 MHz continuously. At 1.1V, the chip draws **~18-20 mA** when both cores are running active polling code.
- **Power State with `__wfi()` Sleep:** When waiting for an event (which is >99.9% of the time in this application), the CPU executes `__wfi()` (Wait For Interrupt). This halts the processor clock tree. The RP2040 current drops to **~1.2 mA** (a **~93% reduction** in CPU core power consumption).
- **Wakeup Latency:** Hardware interrupt wakeup takes only **~15 clock cycles** on the Cortex-M0+ (less than 1 microsecond at 125 MHz). 

Using interrupts matches the latency of RTOS task yields while retaining the low power profile of a sleep state, without any scheduler overhead.

### Implementation
Three interrupt sources wake the processor:

1. **GPIO Edge IRQ** — `GPIO_IRQ_EDGE_FALL` on BTN_PREV (GP26), BTN_NEXT (GP27), BTN_SELECT (GP28).
2. **TinyUSB CDC RX callback** — `tud_cdc_rx_cb()` fires when USB serial data arrives.
3. **DMA Transfer-Complete IRQ** — `DMA_IRQ_0` fires after the full 40,960-byte frame transfer ends.

The main loop checks the three event flags, processes them, and then executes:
```cpp
uint32_t status = save_and_disable_interrupts();
if (!protocol_event_flag && !button_event_flag && !ui_dirty_flag) {
    __wfi(); // CPU sleeps until next interrupt
}
restore_interrupts(status);
```

The `save_and_disable_interrupts()` guard prevents a race condition where an interrupt fires between the flag check and the `__wfi()` call, which would otherwise cause the CPU to sleep with an unprocessed event pending.

### Why Not RTOS?
An RTOS is designed to manage complex multi-threaded scheduling, thread synchronization, and resource locking. In this project, the event execution pipeline is simple and linear:
`Serial/Button Event → Parser/FSM State Update → Render & Queue Frame`.

Since there are no complex blocking operations, shared hardware peripherals requiring lock arbitration, or strict preemptive priority requirements, introducing an RTOS is **over-engineering**. The hand-rolled interrupt-driven architecture delivers identical low-latency and low-power performance with minimal code footprint and no scheduler complexity.

---

## 4. Dynamic FPS Adaptation (Event-Driven vs. Constant Refresh)

### The Problem
A constant refresh rate (e.g., locking the device to 30 or 60 FPS) wastes significant energy and processor resources. For a glanceable dashboard monitor, content remains static for minutes at a time, making a constant display cycle highly inefficient.

### Options Considered

| Approach | Refresh Rate | Power / CPU Idle Overhead | Visual Smoothness |
|---|---|---|---|
| **Fixed Frame Rate (60 FPS)** | Constant 60 Hz | High (Always active rendering) | High |
| **Fixed Frame Rate (1 FPS)** | Constant 1 Hz | Low | Low (Laggy button response) |
| **Dynamic Event-Driven FPS** ✅ | **0 to 73 FPS (on-demand)** | **Zero overhead when static** | **High (Instant on interaction)** |

### Dynamic Adaptation Model
The system architecture adapts the frame rate dynamically across three distinct operational states:

1. **Static Idle State (0 FPS):** When no host data arrives, no buttons are pressed, and no agents are running.
2. **Animated Active State (10 FPS):** If one or more AI agents are in a `RUNNING` status, the display must show an animated spinner. The system dynamically starts a repeating hardware timer interrupt at 100 ms intervals. forcing a stable **10 FPS** refresh rate.
3. **Burst/Interaction State (Up to 73 FPS):** During physical button navigation or intensive serial data streams, the system processes and queues updates instantly.

### Implementation of the Animation Timer (Dynamic 10 FPS)
```cpp
// In ui.cpp
static struct repeating_timer animation_timer;

void ui_update_animation_timer() {
    bool any_running = false;
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (protocol_get_agent(i)->status == AGENT_STATUS_RUNNING) {
            any_running = true; break;
        }
    }
    // Dynamic Timer Lifecycle Management
    if (any_running && !timer_running) {
        add_repeating_timer_ms(UI_ANIM_INTERVAL_MS, ui_timer_callback, NULL, &animation_timer);
        timer_running = true;
    } else if (!any_running && timer_running) {
        cancel_repeating_timer(&animation_timer);
        timer_running = false;
    }
}
```

- **Efficiency when active:** The 100 ms repeating timer callback sets `ui_dirty_flag = true`. The resulting 10 FPS refresh demands a negligible **0.05% CPU active duty cycle** when drawing.
- **Dynamic Cleanup:** The moment all running agents transition to a static state (`DONE`, `ERROR`, `INPUT`), the timer is automatically cancelled via `cancel_repeating_timer()`, instantly dropping the system back to **0 FPS** (WFI sleep).

---

## 5. Backlight Dimming & CPU Dormant (Power Management)

### The Problem
The device is designed to be glanceable, not stared at. Running the LCD backlight at full brightness 24/7 is wasteful on a power souce and would be untenable on battery.

### Design

Two-tier power management:

| Tier | Trigger | Action | Wake Condition |
|---|---|---|---|
| **Active** | Any event | Backlight 100%, CPU processes events | — |
| **Dimmed/Sleep** | 10 s inactivity | `display_set_backlight(0)` | Button press or serial data |

### Implementation
A hardware alarm (`hardware_alarm_claim(0)`) fires `backlight_alarm_handler()` (which calls `display_set_backlight(0)`) 10 seconds after the last interaction.
On wake, `ui_wake_up()` restores full brightness.

### Why a Hardware Alarm Over a Software Counter?
A software counter requires the CPU to be running to decrement it — negating the `__wfi()` sleep. The hardware alarm fires independently of CPU state, allowing the CPU to sleep uninterrupted for the full 10-second window.

### what's not implemented
full rp2040 **dormant mode**(`pico_sleep`) could reduce power from ~20 ma to ~1 ma, but it is unviable for this architecture. shuts down the system plls breaks the usb clock, killing the usb cdc interface required to track host packets. therefore, the design intentionally relies on `__wfi()` to maximize power savings without dropping the host communication link.

---

## 6. Communication Protocol Design

### Requirements
- Carry agent ID, status string, name string, and a free-text message
- Survive serial line noise (garbage bytes on connection/reconnection)
- Support variable-length messages (an agent's log message can be 1 or 63 bytes)
- Be memory-safe: no dynamic allocation

### Options Considered

| Design | Delimiter | Variable Length? | Parsing Complexity |
|---|---|---|---|
| **Newline-delimited JSON** | `\n` | ✅ Yes | High (Requires JSON tokenizer + `malloc`) |
| **Fixed-length binary frames** | None | ❌ No | Low (Direct struct overlay) |
| **Delimiter + escape sequences** | e.g., `\|` | ✅ Yes | Low-Medium (Character-by-character scan) |
| **Length-prefixed binary** ✅ **(chosen)** | `SET:` keyword | ✅ Yes | Low (Staged length-read state machine) |

### Protocol Format
```
SET:<id>:<status_len>:<name_len>:<msg_len>:<payload>
DEL:<id>:
```

**Example:**
```
SET:1:7:7:24:RUNNINGAgent_AProcessing user query...
```

The parser is a 5-state machine:
`WAIT_FOR_CMD → PARSE_CMD → READ_HEADER → READ_PAYLOAD → (commit)`

### Delimiter-Based vs. Length-Prefixed Schemes
Both delimiter-based parsing (e.g. `SET|ID|STATUS|NAME|MESSAGE`) and length-prefixed binary/text framing are highly viable designs for microcontrollers. 

- **Delimiter-Based (Alternative):** Extremely readable in diagnostic terminals. However, it requires character escaping if delimiters (like `|` or `,`) are allowed inside the free-text agent message. Writing escape state-machines adds parsing footprint.
- **Length-Prefixed (Chosen):** Solves the delimiter collision problem naturally because the parser reads exactly $N$ payload bytes without scanning for characters. This creates a highly robust state machine that resists buffer overflows without needing escape parsers. 

Both approaches are valid; we opted for length-prefixed parsing to make payload ingestion deterministic and immune to message contents, while the `WAIT_FOR_CMD` state discards any initial line noise (as validated by `protocol_noise_resilience` testing: `INVALIDCMD!!!SET:1:...` recovers and parses).

---

## 7. Ring Buffer vs. Other Serial RX Buffering Strategies

### The Problem
Although this is a single-threaded bare-metal project, it runs with asynchronous interrupts. The TinyUSB callback `tud_cdc_rx_cb()` fires in **interrupt context (ISR)**, preempting the **main loop** execution thread. These two contexts must share incoming data safely without data corruption or memory races, and without blocking interrupts.

### Options Considered

| Strategy | Interrupt Safe? | Memory | Latency | Notes |
|---|---|---|---|---|
| **Global byte array + `memcpy`** | ❌ No | Fixed | Low | Race condition if ISR preempts during copy |
| **Mutex / Disable Interrupts** | ⚠️ Risky | Fixed | High | Stalling interrupts in USB ISR causes packet loss |
| **Lock-free Ring Buffer** ✅ | **✅ Yes (SPSC)** | **Fixed** | **Very Low** | **No locks or interrupt disabling needed** |

### Why the Ring Buffer Is Safe Without Locks
The ring buffer uses a **single-producer / single-consumer (SPSC)** architecture:
- **Producer (USB ISR):** Only writes to `head` and reads `tail` (to check if full).
- **Consumer (Main Loop):** Only writes to `tail` and reads `head` (to check if data is available).

### Sizing
```
RING_BUFFER_SIZE = 512 bytes
```
- Worst-case packet: `SET:6:7:15:63:<payload>` = ~4 (header keyword) + ~15 (length fields) + 7 + 15 + 63 = **~104 bytes**
- Buffer holds **~4 max-length packets** simultaneously — providing transient headroom for burst host messages.
- Memory cost: 512 bytes + 8 bytes control variables = **520 bytes**.

---

