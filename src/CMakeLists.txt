pico_sdk_init()
add_executable(ddsTest
    main.c
    ddsInterface.c
    cmdParser.c
)

target_link_libraries(ddsTest PRIVATE
    pico_multicore
    pico_stdlib
    hardware_pio
    hardware_dma
)

pico_generate_pio_header(ddsTest ${CMAKE_CURRENT_LIST_DIR}/fastDAC.pio)

pico_enable_stdio_uart(ddsTest 1)