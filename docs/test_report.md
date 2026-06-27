# Test Results: Agent Monitor Embedded System

**Test Suite:** `tests/`  
**Last Updated:** 2026-06-27

---

## 1. Overview

The test suite is a **native host-compiled** unit test harness that isolates each module under test from its hardware dependencies using lightweight hand-rolled mocks. All tests are compiled and executed on the development machine (Linux/macOS).

The suite covers **4 modules** across **20 test cases** and **~100+ assertions**.

---

## 2. Test Infrastructure

### 2.1 Custom Test Framework (`test_framework.h`)

A minimal, header-only framework with no external dependencies. It provides:

| Macro / Function | Purpose |
|---|---|
| `ASSERT_TRUE(expr)` | Fails if `expr` is falsy |
| `ASSERT_FALSE(expr)` | Fails if `expr` is truthy |
| `ASSERT_EQ(actual, expected)` | Fails if values differ; prints both sides |
| `ASSERT_STR_EQ(actual, expected)` | String equality with human-readable diffs |
| `run_test_case(tc, passed, total)` | Runs a single `TestCase`, catches C++ exceptions |

Global counters (`total_assertions`, `failed_assertions`, `current_test_failed`) track pass/fail state across the entire run.

### 2.2 Test Runner (`test_runner.cpp`)

The `main()` entry point collects all test cases from each suite's `register_*_tests()` function, runs them sequentially, and prints a colored summary:

```
==================================================
             Agent Monitor Unit Tests
==================================================
[ RUN      ] ring_buffer_init
[       OK ] ring_buffer_init
...
==================================================
Test Summary:
  Passed Tests:      20 / 20
  Total Assertions:  104
  Failed Assertions: 0
==================================================
Result: ALL TESTS PASSED!
```

Exit code `0` on full pass, `1` on any failure.

### 2.3 Build System (`tests/CMakeLists.txt`)

The test suite uses a **standalone CMake project** (`agent_monitor_tests`) separate from the Pico firmware build. It compiles directly against the host C++ toolchain (C++17).

**Sources under test:**

| Source File | Module |
|---|---|
| `src/ring_buffer.cpp` | Ring Buffer |
| `src/protocol.cpp` | Serial Protocol Parser |
| `src/button.cpp` | Button / GPIO Handler |
| `src/ui.cpp` | UI State Machine |
| `src/font.cpp` | 5x7 Font Bitmap Table |

**Include path ordering** (critical): `tests/mocks/` is listed **before** `include/` so mock headers shadow the real Pico SDK headers.

---

## 3. Mock Infrastructure (`tests/mocks/`)

Since the production code calls Pico SDK and TinyUSB APIs, each hardware dependency is replaced with a controllable software stub.

### 3.1 `mock_peripherals.cpp`

Provides the backing state for all peripheral mocks:

| Mock Variable | Type | Purpose |
|---|---|---|
| `mock_absolute_time_us` | `uint64_t` | Simulated monotonic clock for button debounce |
| `mock_gpio_states[32]` | `bool[]` | GPIO pin read values |
| `mock_gpio_dirs[32]` | `uint[]` | GPIO pin direction |
| `mock_gpio_pullups[32]` | `bool[]` | GPIO pull-up configuration |
| `mock_gpio_irq_callback` | function ptr | Captured GPIO IRQ handler |
| `mock_cdc_rx_data` | `vector<uint8_t>` | Data delivered by the USB CDC RX callback |
| `mock_cdc_rx_idx` | `size_t` | Cursor into `mock_cdc_rx_data` |

**Mocked SDK headers** (shadow real Pico SDK):

| Mock Header | Real API Replaced |
|---|---|
| `pico/time.h` | `get_absolute_time()`, `absolute_time_diff_us()` |
| `hardware/gpio.h` | `gpio_init()`, `gpio_set_irq_enabled_with_callback()`, etc. |
| `hardware/spi.h` | SPI stubs (no-ops) |
| `hardware/sync.h` | Spinlock stubs (no-ops) |
| `hardware/timer.h` | Alarm stubs (no-ops) |
| `tusb.h` | `tud_cdc_available()`, `tud_cdc_read()`, `tud_cdc_rx_cb()` |

### 3.2 `mock_display.cpp`

Replaces the hardware SPI/DMA display driver with an **in-memory pixel buffer**:

| Mock Function | Behavior |
|---|---|
| `display_init()` | Zeroes both `buffer_0` / `buffer_1`; sets backlight to max |
| `display_clear(color)` | Fills `back_buffer` with a solid color |
| `display_draw_pixel/rect/char/string()` | Writes pixels into `back_buffer` |
| `display_flush_async()` | Synchronously swaps `front_buffer` <-> `back_buffer` pointers (no DMA) |
| `display_is_busy()` | Returns controllable `mock_display_busy` flag |
| `display_wait_ready()` | No-op |

Tests can inspect `front_buffer[]` and `back_buffer[]` directly to verify render output.

---

## 4. Test Suites

### 4.1 Ring Buffer (`test_ring_buffer.cpp`)

Tests the thread-safe circular byte buffer used to receive incoming USB CDC serial data.

| # | Test Case | Description | Key Assertions |
|---|---|---|---|
| 1 | `ring_buffer_init` | A freshly initialized buffer is empty; pop on empty returns `false` | `is_empty == true`, `pop == false` |
| 2 | `ring_buffer_push_pop` | Push a single byte, verify it can be popped with the correct value, and the buffer becomes empty again | `data == 42`, `is_empty == true` after pop |
| 3 | `ring_buffer_fifo_order` | Push 10 sequential values; verify they emerge in FIFO order | Values 0-9 pop in order |
| 4 | `ring_buffer_full_condition` | Fill to max capacity (`RING_BUFFER_SIZE - 1 = 511` bytes); verify the 512th push fails; pop one and verify a new push succeeds; drain and confirm correct ordering | Overflow returns `false`; recovery and subsequent data integrity verified |

**Module:** `src/ring_buffer.cpp` — `RING_BUFFER_SIZE = 512`

---

### 4.2 Serial Protocol (`test_protocol.cpp`)

Tests the two-stage state-machine parser that decodes length-prefixed `SET`/`DEL` packets from the ring buffer into `AgentData` structures.

**Packet format:** `SET:<id>:<status_len>:<name_len>:<msg_len>:<payload>`  
**Example:** `SET:1:7:7:10:RUNNINGAgent_AProcessing`

| # | Test Case | Description | Key Assertions |
|---|---|---|---|
| 5 | `protocol_init` | All `MAX_AGENTS` (7) slots initialize with correct IDs, `is_active=false`, `is_dirty=false` | All slots inactive after init |
| 6 | `protocol_valid_set` | Parse a well-formed `SET` packet for agent 0 | `is_active`, status `RUNNING`, name `"Agent_A"`, message `"Processing"`, `is_dirty=true`, `selected_idx=0` |
| 7 | `protocol_valid_del` | Add agent 3 via `SET`, then remove with `DEL:3:`; verify slot cleared and `selected_idx` reset | `is_active=false` after DEL, `selected_idx=-1` |
| 8 | `protocol_noise_resilience` | Prefix a valid `SET` packet with garbage bytes (`INVALIDCMD!!!`); verify the parser recovers and correctly processes the valid command | Agent data populated correctly despite leading junk |
| 9 | `protocol_invalid_id` | Send `SET:9:...` where ID 9 exceeds `MAX_AGENTS=7` | No crash; all agent slots remain inactive |
| 10 | `protocol_buffer_overflow_protection` | Send a name field declared as 30 bytes (exceeds `MAX_NAME_LEN=16`); verify safe truncation to 15 chars + null terminator | `name == "LongAgentNameTh"`, no buffer overrun |

**Module:** `src/protocol.cpp` — depends on ring buffer; uses `tud_cdc_rx_cb()` mock to inject data

---

### 4.3 UI State Machine (`test_ui.cpp`)

Tests the finite-state machine (FSM) governing navigation, button handling, and display rendering. Uses mock time (`mock_absolute_time_us`) to control debounce behavior and inspects `front_buffer[]` for pixel-level render verification.

**Helper:** `press_button(gpio)` — advances mock clock by 250 ms (past the 200 ms debounce window) then calls `handle_button_event()`.

| # | Test Case | Description | Key Assertions |
|---|---|---|---|
| 11 | `ui_init` | UI starts in `STATE_LIST` with no agent selected and zero scroll offset | `current_state == STATE_LIST`, `selected_idx == -1`, `scroll_offset == 0` |
| 12 | `ui_carousel_cycling` | Add 2 agents (IDs 1 and 4); press `BTN_NEXT` to cycle forward; press again to wrap around; press `BTN_PREV` to cycle back | `selected_idx` transitions: 1 -> 4 -> 1 -> 4 |
| 13 | `ui_state_transition` | With agent at ID 2, press `BTN_SELECT` to enter `STATE_DETAIL`; press again to return to `STATE_LIST`; verify `scroll_offset` resets on entry | State toggles cleanly; scroll offset resets to 0 |
| 14 | `ui_scrolling` | In `STATE_DETAIL`, `BTN_NEXT` scrolls down by `UI_DETAIL_SCROLL_STEP` per press; `BTN_PREV` scrolls up; `BTN_PREV` at offset 0 is clamped (no negative scroll) | Offset increments/decrements correctly; clamped at 0 |
| 15 | `ui_button_debouncing` | Press `BTN_NEXT` (advances time); verify selection changes. Then immediately call `handle_button_event()` without advancing time; verify second press is ignored | `selected_idx` unchanged on rapid re-press |
| 16 | `ui_rendering` | Set `ui_dirty_flag = true`, call `ui_update()`; after buffer swap, inspect `front_buffer` pixels: top row must be yellow header bar; lower pixel must be black background | `front_buffer[0 * LCD_WIDTH + x] == COLOR_YELLOW`; `front_buffer[80 * LCD_WIDTH + 10] == COLOR_BLACK` |

**Module:** `src/ui.cpp`, `src/button.cpp` — depends on protocol and mock display

---

### 4.4 Font Bitmap Table (`test_font.cpp`)

Tests the hardcoded 5x7 monochrome font table (`font_5x7[]`) that maps ASCII characters (32-126) to 5-column bitmap bytes.

| # | Test Case | Description | Key Assertions |
|---|---|---|---|
| 17 | `font_ascii_range` | Space character (index 0) must be all-zero columns; `'!'` (index 1) must have at least one non-zero column | Space is blank; `!` has rendered pixels |
| 18 | `font_glyph_A` | Verify exact bitmap bytes for `'A'` (index 33): `{0x7E, 0x11, 0x11, 0x11, 0x7E}` | All 5 column bytes match exactly |
| 19 | `font_glyph_zero` | Verify exact bitmap bytes for `'0'` (index 16): `{0x3E, 0x51, 0x49, 0x45, 0x3E}` | All 5 column bytes match exactly |
| 20 | `font_symmetry` | Spot-check that symmetric characters (`A H I M O T U V W X`) have matching outer columns (`col[0]==col[4]`) and inner columns (`col[1]==col[3]`) | Left-right symmetry holds for all listed glyphs |

**Module:** `src/font.cpp` — no hardware dependencies; pure data table verification

---

## 5. Test Coverage Summary

| Suite | File | Tests | Focus Area |
|---|---|---|---|
| Ring Buffer | `test_ring_buffer.cpp` | 4 | Init, push/pop, FIFO order, full/overflow |
| Protocol | `test_protocol.cpp` | 6 | Init, SET/DEL packets, noise resilience, bounds checking |
| UI | `test_ui.cpp` | 6 | FSM states, carousel, scrolling, debounce, pixel rendering |
| Font | `test_font.cpp` | 4 | ASCII range, specific glyphs, structural symmetry |
| **Total** | | **20** | |

---

## 6. Building & Running Tests

The test suite is compiled independently from the Pico firmware using the host C++ toolchain.

```bash
# From the project root
cd tests
mkdir -p build && cd build
cmake ..
make -j4
./run_tests
```

> **Note:** The test CMake project (`tests/CMakeLists.txt`) does **not** use `pico_sdk_import.cmake` and does **not** cross-compile. It targets the local host machine directly.

> **Important:** The `tests/mocks/` include directory **must** appear before `include/` in the compiler's include search path. This is enforced in `CMakeLists.txt` so that mock headers shadow the real Pico SDK headers.
