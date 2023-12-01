/**
 *  DDS main app
 */
#include <stdio.h>
#include <string.h>
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/vreg.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "common_configs.h"
#include "fastDAC.pio.h"
#include "ddsInterface.h"

char __attribute__((aligned(4)))  ddsBuffer1[DDS_BUFFER_SIZE];
char __attribute__((aligned(4)))  ddsBuffer2[DDS_BUFFER_SIZE];
uint sampling_rate = 1;
char update_system = 0;

void VCORE_OVC_RUN_IN_RAM core1_main() {
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1);
    printf("Core 1 has started\n");
    
    //Initial DDS waveform
    set_dds_buffer_sine(ddsBuffer1, DDS_BUFFER_SIZE, DAC_RANGE, 1);
    set_dds_buffer_copy(ddsBuffer1, ddsBuffer2, DDS_BUFFER_SIZE, 8);

    //Allow Core 0 to continue
    multicore_fifo_push_blocking((uint32_t)1);

    //Wait Core 0 message
    printf((const char*) multicore_fifo_pop_blocking());

    //Show Available commands
    print_dds_cmds();

    int argc;
    char *argv[16];
    char inputStr[128];
    char *buffers[2] = { ddsBuffer1, ddsBuffer2 };
    
	while (1) {
        gets(inputStr);
        get_argv_argc(inputStr, &argc, argv);
        
        parse_dds_command(argc, argv, buffers, &sampling_rate, DDS_BUFFER_SIZE, DAC_RANGE);
	}
	__builtin_unreachable();
}

uint prepare_dma_pio_dac(PIO pio, uint samplingRate, uint pinBase, void *dmaBuffer) {
    //Prepare PIO State Manager
    uint sm = pio_claim_unused_sm(pio, true);
    int offset = pio_add_program(pio, &fast_dac_program);
    fast_dac_program_init(pio, sm, offset, samplingRate, pinBase);

    //Prepare DMA
    int dma_channel = dma_claim_unused_channel(true);
    dma_channel_config dma_channel_cfg = dma_channel_get_default_config(dma_channel);
    channel_config_set_transfer_data_size(&dma_channel_cfg, DMA_SIZE_32); //Transfer 4 8bits words that are shifted by pio
    channel_config_set_dreq(&dma_channel_cfg, pio_get_dreq(pio, sm, true));
    channel_config_set_read_increment(&dma_channel_cfg, true);
    channel_config_set_write_increment(&dma_channel_cfg, false);

    dma_channel_configure(dma_channel,
        &dma_channel_cfg,
        &pio->txf[sm],
        dmaBuffer,
        DDS_BUFFER_SIZE>>2,  
        false
    );
    return dma_channel;
}

int VCORE_OVC_RUN_IN_RAM main() {
    VCORE_OVC_PREPARE(256000); // Ignored if USE_VCORE_OVC is disabled

    stdio_init_all();
    //stdio_uart_init_full(uart_default, PICO_STDIO_UART_BAUDS, PICO_STDIO_UART_TX_PIN, PICO_STDIO_UART_RX_PIN);
   
    printf("Core 0 has started\n");
    multicore_launch_core1(core1_main);

    //Add a simple test for multicore communication, wait Core 1 says it's ok to start
    uint32_t run = 0;
    while (!run) {
        run = multicore_fifo_pop_blocking();
    }

    //Send a message back to Core 1
    multicore_fifo_push_blocking((uint32_t)"Core 0 DDS starting\n");

    //Prepare the Pio State manager
    PIO pio = pio0;
    int dma_channel1 = prepare_dma_pio_dac(pio, clock_get_hz(clk_sys), FAST_DAC1_PIN_BASE, ddsBuffer1);
    int dma_channel2 = prepare_dma_pio_dac(pio, clock_get_hz(clk_sys), FAST_DAC2_PIN_BASE, ddsBuffer2);
    while (1) {
        while (!update_system) {
            dma_channel_wait_for_finish_blocking(dma_channel1);
            dma_channel_hw_addr(dma_channel1)->al3_read_addr_trig = (uintptr_t) ddsBuffer1;
            dma_channel_hw_addr(dma_channel2)->al3_read_addr_trig = (uintptr_t) ddsBuffer2;
        }
        update_system = 0;
        
    }
    __builtin_unreachable();
}

