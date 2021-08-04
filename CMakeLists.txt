cmake_minimum_required(VERSION 3.13)
# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)
project(pico_arcade)
 
pico_sdk_init()

add_executable(pico_arcade)

# target_compile_definitions(pico_arcade
#   PUBLIC PICO_UART_ENABLE_CRLF_SUPPORT=0
#   PUBLIC PICO_STDIO_ENABLE_CRLF_SUPPORT=0
#   PUBLIC PICO_STDIO_DEFAULT_CRLF=0
# )

target_sources(pico_arcade PRIVATE
        main.c
        led.c
        button.c
        )

target_link_libraries(pico_arcade PRIVATE pico_stdio pico_stdlib pico_multicore pico_time pico_sync)
pico_add_extra_outputs(pico_arcade)

# enable usb output, disable uart output
pico_enable_stdio_usb(pico_arcade 1)
pico_enable_stdio_uart(pico_arcade 0)
