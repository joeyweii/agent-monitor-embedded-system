cmake_minimum_required(VERSION 3.13)

# Generate compile_commands.json for LSP (neovim/clangd)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set PICO_SDK_PATH to the submodule directory
set(PICO_SDK_PATH ${CMAKE_CURRENT_SOURCE_DIR}/pico-sdk)

# Include Pico SDK import
include(pico_sdk_import.cmake)

project(agent_monitor C CXX ASM)

# Initialize Pico SDK
pico_sdk_init()

add_executable(agent_monitor
    src/main.cpp
    src/display.cpp
    src/button.cpp
    src/font.cpp
    src/protocol.cpp
    src/ui.cpp
    src/ring_buffer.cpp
)

# Include headers
target_include_directories(agent_monitor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Set compile options
target_compile_features(agent_monitor PRIVATE cxx_std_17)

# Link hardware libraries
target_link_libraries(agent_monitor PRIVATE
    pico_stdlib
    hardware_spi
    hardware_gpio
    hardware_dma
    hardware_pwm
)

# Enable USB serial output (stdio over USB), disable UART output
pico_enable_stdio_usb(agent_monitor 1)
pico_enable_stdio_uart(agent_monitor 0)

# Create map/bin/hex/uf2 file formats
pico_add_extra_outputs(agent_monitor)
