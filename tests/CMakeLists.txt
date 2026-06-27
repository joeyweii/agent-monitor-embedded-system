cmake_minimum_required(VERSION 3.13)
project(agent_monitor_tests CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files to test
set(SRC_UNDER_TEST
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ring_buffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/protocol.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/button.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/font.cpp
)

# Test runner files and mocks
set(TEST_FILES
    test_runner.cpp
    test_ring_buffer.cpp
    test_protocol.cpp
    test_ui.cpp
    test_font.cpp
    mocks/mock_peripherals.cpp
    mocks/mock_display.cpp
)

# Executable target
add_executable(run_tests ${SRC_UNDER_TEST} ${TEST_FILES})

# Include directories
# Note: ${CMAKE_CURRENT_SOURCE_DIR}/mocks MUST come first so mocked headers are preferred.
target_include_directories(run_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}
)
