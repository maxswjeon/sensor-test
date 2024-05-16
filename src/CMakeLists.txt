file(GLOB_RECURSE FILES *.c *.cpp *.h *.hpp *.pio)

add_executable(${PROJECT} ${FILES} ${LIB_SOURCES})

pico_enable_stdio_usb(${PROJECT} 1)
pico_enable_stdio_uart(${PROJECT} 0)
pico_add_extra_outputs(${PROJECT})

target_include_directories(${PROJECT} PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_ROOT_DIR}/include
    ${CMAKE_ROOT_DIR}/lib
    ${CMAKE_ROOT_DIR}
)

target_link_libraries(${PROJECT}
    pico_stdlib
    pico_cyw43_arch_none
    pico_bootrom
    pico_malloc
    pico_runtime
    pico_multicore
    pico_sync
    hardware_pwm
    hardware_i2c
    hardware_spi
    hardware_watchdog
)
