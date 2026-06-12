cmake_minimum_required(VERSION 3.13)

# Generate compile_commands.json for LSP (neovim/clangd)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set PICO_SDK_PATH to the submodule directory
set(PICO_SDK_PATH ${CMAKE_CURRENT_SOURCE_DIR}/pico-sdk)

# Include Pico SDK import
include(pico_sdk_import.cmake)

project(hardware_test C CXX ASM)

# Initialize Pico SDK
pico_sdk_init()

add_executable(hardware_test
    main.cpp
)

# Set compile options
target_compile_features(hardware_test PRIVATE cxx_std_17)

# Link hardware libraries
target_link_libraries(hardware_test PRIVATE
    pico_stdlib
    hardware_spi
    hardware_gpio
)

# Enable USB serial output (stdio over USB), disable UART output
pico_enable_stdio_usb(hardware_test 1)
pico_enable_stdio_uart(hardware_test 0)

# Create map/bin/hex/uf2 file formats
pico_add_extra_outputs(hardware_test)
