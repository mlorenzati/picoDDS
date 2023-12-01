#ifndef _COMMON_CONFIGS_H
#define _COMMON_CONFIGS_H

// USART PINs
#define PICO_STDIO_UART_TX_PIN 16
#define PICO_STDIO_UART_RX_PIN 17
#define PICO_STDIO_UART_BAUDS  115200

// R2R DACs
#define FAST_DAC1_PIN_BASE   0
#define FAST_DAC1_BITS       5
#define FAST_DAC2_PIN_BASE   6
#define FAST_DAC2_BITS       5
#define FAST_DAC3_PIN_BASE   11
#define FAST_DAC3_BITS       5

//
#define TRUE  1 
#define FALSE 0 

//Buffers
#define DAC_RANGE (1<< FAST_DAC1_BITS)
#define DDS_BUFFER_SIZE DAC_RANGE

// VCore clocking

#define USE_VCORE_OVC               TRUE
#define VREG_VSEL                   VREG_VOLTAGE_1_20
#if USE_VCORE_OVC == TRUE
    #define VCORE_OVC_RUN_IN_RAM    __not_in_flash("main")
    #define VCORE_OVC_PREPARE(x)    vreg_set_voltage(VREG_VSEL); sleep_ms(10); set_sys_clock_khz(x, true);
#else
    #define VCORE_OVC_RUN_IN_RAM
    #define VCORE_OVC_PREPARE(x)    
#endif

#endif
