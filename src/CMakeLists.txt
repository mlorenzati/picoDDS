pico_sdk_init()
add_executable(picoDDS
    main.c
    ddsInterface.c
    cmdParser.c
)

target_link_libraries(picoDDS PRIVATE
    pico_multicore
    pico_stdlib
    hardware_pio
    hardware_dma
)

pico_add_extra_outputs(picoDDS)

pico_generate_pio_header(picoDDS ${CMAKE_CURRENT_LIST_DIR}/fastDAC.pio)

pico_enable_stdio_usb(picoDDS 1)
pico_enable_stdio_uart(picoDDS 0)