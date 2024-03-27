file(GLOB_RECURSE FILES *.c *.cpp *.h *.hpp *.pio)
file(GLOB_RECURSE LIB_SOURCES
    ${CMAKE_ROOT_DIR}/lib/*.c
    ${CMAKE_ROOT_DIR}/lib/*.cpp
    ${CMAKE_ROOT_DIR}/lib/*.h
    ${CMAKE_ROOT_DIR}/lib/*.hpp
)

add_executable(${PROJECT} ${FILES} ${LIB_SOURCES})

pico_enable_stdio_usb(${PROJECT} 1)
pico_enable_stdio_uart(${PROJECT} 0)
pico_add_extra_outputs(${PROJECT})

target_include_directories(${PROJECT} PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_ROOT_DIR}/include
    ${CMAKE_ROOT_DIR}/lib
)

target_link_libraries(${PROJECT}
    pico_stdlib
    pico_runtime
    pico_cyw43_arch_none
    hardware_spi
    hardware_clocks
    hardware_watchdog
)
