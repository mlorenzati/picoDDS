#include "ddsInterface.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "math.h"

option options[] =
{
    {"add",       TRUE, NULL, 'a'},
    {"channel",   TRUE, NULL,  'c'},
    {"frequency", TRUE, NULL,  'f'},
    {"multiply",  TRUE, NULL,  'm'},
    {"offset",    TRUE, NULL,  'o'},
    {"reset",     FALSE, NULL, 'r'},
    {"waveform",  TRUE, NULL,  'w'},    
    {NULL,        FALSE, NULL,  0 }
};

char dds_channel = 0;
char cycles = 1;
char *type;
char *cycleStr;
char offset;
float multiply = 0;

void set_dds_buffer_sine(char *buffer, char samples, char dacRange, char cycles) {
    // Cycles have to be a 2^N factor to be a complete wave
    for (char i=0; i < samples; i++) {
        buffer[i] = sin((2*cycles*M_PI*i)/samples)*dacRange/2+(dacRange/2)-1;
    }
}

void set_dds_buffer_square(char *buffer, char samples, char dacRange, char cycles) {
    // Cycles have to be a 2^N factor to be a complete wave
    char trigger = samples / (cycles * 2);
    char cnt = trigger;
    char output = dacRange - 1;
    for (char i=0; i < samples; i++) {
        if (--cnt == 0) {
           output = output > 0 ? 0 : dacRange - 1;
           cnt = trigger;
        }
        buffer[i] = output;
    }
}

void set_dds_buffer_triangle(char *buffer, char samples, char dacRange, char cycles) {
    signed char cnt = 0;
    char up_down = 1;
    for (char i=0; i < samples; i++) {
        buffer[i] =  cnt;
        if (up_down) {
           cnt += 2 * cycles;
           if (cnt >= (dacRange - 1)) { 
               up_down = ! up_down;
               cnt = dacRange - 1;
            }
        } else {
            cnt -= 2 * cycles;
            if (cnt <= 0) { 
                up_down = ! up_down;
                cnt = 0;
            }
        }
    }
}

void set_dds_buffer_saw(char *buffer, char samples, char dacRange, char cycles) {
    char cnt = 0;
    char trigger = samples / cycles;

    for (char i=0; i < samples; i++) {
        buffer[i] =  cycles * cnt;
        if (++cnt >= trigger) { cnt = 0;}
    }
}

void set_dds_buffer_noise(char *buffer, char samples, char dacRange) {
    for (char i=0; i < samples; i++) {
        buffer[i] = rand() & (dacRange - 1);
    }
}

void set_dds_buffer_copy(char *src, char *dst, char samples, char offset) {
    char cnt = offset;
    char __attribute__((aligned(4)))  copyBuffer[64];
    if (src == dst) {
        set_dds_buffer_copy(src, copyBuffer, samples, offset);
        offset = 0;
        dst = src;
        src = copyBuffer;
    }
    for (char i=0; i < samples; i++) {
        dst[i] = src[cnt++];
        if (cnt >= samples) { cnt = 0; }    
    }
}

void set_dds_buffer_add(char *src1, char *src2, char *dst, char samples) {
    for (char i=0; i < samples; i++) {
        dst[i] = (char)((signed char)(src1[i]-15) + (signed char)(src2[i]-15))+15;    
    }
}

void set_dds_buffer_multiply(char *src1, char *src2, float constant, char *dst, char samples) {
    char peak = 0;
    for (char i=0; i < samples; i++) {
        char src2_opt = (src2 == NULL) ? 1 : (src2[i]-15);
        dst[i] = ((float)(src1[i]-15) * (float)src2_opt * constant)+15;
    }
}

struct repeating_timer timer;
char   noiseTimer = 0;
typedef struct  {
    char **buffer;
    char samples;
    char dacRange;
} timer_data;

timer_data data;

bool noise_timer_callback(struct repeating_timer *t) {
    timer_data *data = (timer_data *)(t->user_data);
    if (noiseTimer & 0x1) {
        set_dds_buffer_noise(data->buffer[0], data->samples, data->dacRange);
    }
    if (noiseTimer & 0x2) {
        set_dds_buffer_noise(data->buffer[1], data->samples, data->dacRange);
    }
    return true;
}

void parse_dds_command(int argc, char *const argv[], char *buffer[], unsigned int *sampling_rate, char samples, char dacRange) {
    int option_result = 0;
    int option_index = 0;
    int arg_index = 0;
    char *strValue = NULL;

    while ((option_result = get_cmd(argc, argv, options, &option_index, &arg_index)) != -1) {
        strValue = options[option_index].strval;
        printf ("Request %s<%c>(%s)\n", options[option_index].name, option_result, strValue);

        switch (option_result) {
                case 'a':  
                    dds_channel = atoi(strValue) > 0 ? 1 : 0;
                    printf("-add channels into channel %d\n", dds_channel);
                    set_dds_buffer_add(buffer[0], buffer[1], buffer[dds_channel], samples);
                    break;
                case 'c':  
                    dds_channel = atoi(strValue) > 0 ? 1 : 0;
                    printf("-Selected channel %d\n", dds_channel);
                    break;
                case 'f':
                    *sampling_rate = atoi(strValue);
                    printf("-Change frequency to %d\n", *sampling_rate);
                    break;
                case 'm':
                    multiply = atof(strValue);
                    if (multiply == 0) {
                        printf("-Multiply each channel into %d\n", dds_channel);
                        set_dds_buffer_multiply(buffer[0], buffer[1], 1/dacRange, buffer[dds_channel], samples);
                    } else {
                        printf("-Multiply channel %d to constant %f\n", dds_channel, multiply);
                        set_dds_buffer_multiply(buffer[dds_channel], NULL, multiply, buffer[dds_channel], samples);
                    }
                    break;
                case 'o':
                    offset = atoi(strValue);
                    printf("-Change offset to %d\n", offset);
                    set_dds_buffer_copy(buffer[dds_channel], buffer[dds_channel], samples, offset);
                    break;
                case 'r': printf("-Reset requested\n"); 
                    break;
                case 'w': 
                    type = strtok(strValue, ",");
                    if (type = NULL) {
                        printf("-Waveforms: sine, square, triangle, saw, noise. Optional: ,<cycles>\n");
                    } else {
                        cycleStr = strtok(NULL, ",");
                        if (cycleStr == NULL) { cycleStr = "1"; }
                        cycles = atoi(cycleStr);
                        
                        noiseTimer &= dds_channel ? 0x02 : 0x01;
                        if (noiseTimer == 0) { cancel_repeating_timer(&timer); }

                        printf("-Change wave to %s, cycles %s\n",  strValue, cycleStr);
                        if        (strcmp(strValue, "sine") == 0) {
                            set_dds_buffer_sine(buffer[dds_channel], samples, dacRange, cycles);
                        } else if (strcmp(strValue, "square") == 0) {
                            set_dds_buffer_square(buffer[dds_channel], samples, dacRange, cycles);
                        } else if (strcmp(strValue, "triangle") == 0) {
                            set_dds_buffer_triangle(buffer[dds_channel], samples, dacRange, cycles);
                        } else if (strcmp(strValue, "saw") == 0) {
                            set_dds_buffer_saw(buffer[dds_channel], samples, dacRange, cycles);
                        } else if (strcmp(strValue, "noise") == 0) {
                            add_repeating_timer_us(200, noise_timer_callback, NULL, &timer);
                            noiseTimer |= 1 << dds_channel;
                            data.buffer = buffer;
                            data.dacRange = dacRange;
                            data.samples = samples;
                            timer.user_data = &data;
                        } else { printf("-Unsupported waveform %s\n", strValue); }

                        
                    } 
                    break;
                default:  printf("Unknown command: "); print_cmds(options); break;
        }
    }
    printf("Request completed\n");
}
    
