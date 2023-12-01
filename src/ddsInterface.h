#ifndef DDS_INTERFACE
#define DDS_INTERFACE
#include "cmdParser.h"
extern option options[];

#define print_dds_cmds() print_cmds(options);

void set_dds_buffer_sine(char *buffer, char samples, char dacRange, char cycles);
void set_dds_buffer_copy(char *src, char *dst, char samples, char offset);
void parse_dds_command(int argc, char *const argv[], char *buffer[], unsigned int *sampling_rate, char samples, char dacRange);

#endif