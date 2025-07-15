# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.0.0)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.0.0)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(level C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_BOARD adafruit_feather_rp2040)

pico_sdk_init()

# Define the executable and source files
add_executable(level src/level.cpp libraries/LIS3DH.cpp libraries/neomatrix.cpp)

target_link_libraries(level pico_stdlib hardware_i2c hardware_pio)


pico_generate_pio_header(level ${CMAKE_CURRENT_LIST_DIR}/libraries/ws2812.pio
        OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/libraries)

pico_add_extra_outputs(level)


# Link required libraries
pico_enable_stdio_usb(level 1)
pico_enable_stdio_uart(level 1)