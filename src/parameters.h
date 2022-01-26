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
    ePARAM_LABEL_PATTERN,
    ePARAM_len
};

enum
{
    eUNDEF_ENDIAN,
    eLITTLE_ENDIAN,
    eBIG_ENDIAN
};

//Return negative value on error, positive on warnigs and 0 on success
int command_param(linked_list_t* args);