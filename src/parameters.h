#pragma once

#include <stdlib.h>
#include <stdint.h>

#include "linked_list.h"

// Could implement it as a perfect hash table using gperf, but not a priority
enum
{
    ePARAM_OPCODE_WIDTH,
    ePARAM_MEMORY_WIDTH,
    ePARAM_ALIGNMENT,
    ePARAM_ADDRESS_WIDTH,
    ePARAM_ADDRESS_START,
    ePARAM_ADRRESS_STOP,
    ePARAM_ENDIANNESS,
    ePARAM_ARGS_SEPARATOR,
    ePARAM_LABEL_POSTFIX,
    ePARAM_CONSTANT_DIRECTIVE,
    ePARAM_AUTHOR,
    ePARAM_NAME,
    ePARAM_VERSION,
    ePARAM_COPYRIGHT,
    ePARAM_len
};

enum
{
    eUNDEF_ENDIAN = -1,
    eBIG_ENDIAN = 0,
    eLITTLE_ENDIAN
};

typedef struct
{
    // Memory parameters, all mandatory
    int64_t opcode_width;  // The width of the opcode
    int64_t memory_width;  // The width of the memory
    int64_t alignment;     // Memory alignment, if none specified it will align to the opcode width
    int64_t address_width; // The address bus width
    int64_t address_start; // Address where the memory starts. Inclusive
    int64_t address_stop;  // Address where the memory stops. Inclusive
    int endianness;        // Endianness. If little endian the bit order is reversed

    // Syntax parameters, all optional with default values
    char args_separator; // The character used to separate the arguments of a mnemonic
    char label_postfix; // The character at the end of a label declaration (':' by default)
    char* constant_dir; // Directive for constant declaration

    // Version parameters, all optional
    char* author;
    char* version;
    char* name;
    char* copyright;
} parameters_t;

extern parameters_t parameters;

void param_init();
void param_fill_unset();

//Return negative value on error, positive on warnigs and 0 on success
int command_param(linked_list_t* args);