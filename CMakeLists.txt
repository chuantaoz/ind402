# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

include(pico_extras_import.cmake)

project(IND402_1 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(IND402_1 IND402_1.c )



pico_set_program_name(IND402_1 "IND402_1")
pico_set_program_version(IND402_1 "0.1")

pico_enable_stdio_uart(IND402_1 1)
pico_enable_stdio_usb(IND402_1 0)

pico_generate_pio_header(IND402_1 ${CMAKE_CURRENT_LIST_DIR}/blink.pio)
target_sources(IND402_1 PRIVATE IND402_1.c)

# Add the standard library to the build
target_link_libraries(IND402_1 pico_stdlib)

# Add any user requested libraries
target_link_libraries(IND402_1
        hardware_dma
        hardware_pio
        #hardware_rosc
        hardware_sleep
        hardware_timer
        hardware_clocks
        hardware_pwm
        )

pico_enable_stdio_usb(IND402_1 1)
#pico_enable_stdio_uart(IND402_1 0)

pico_add_extra_outputs(IND402_1)

#pico_set_binary_type(<YourProjectName> no_flash)

