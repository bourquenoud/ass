#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "macro.h"
#include "xmalloc.h"
#include "commands.h"
#include "hash_array.h"
#include "ast_node.h"

typedef enum
{
    eBP_UNDEF = 0,
    eBP_ID,
    eBP_IMMEDIATE,
    eBP_LABEL_ABS,
    eBP_LABEL_REL,
    eBP_ENUM,
    eBP_BIT_CONST,
    eBP_BIT_LIT,
    eBP_ELLIPSIS,
} BPTYPE_t;


typedef struct bit_elem_s
{
    BPTYPE_t type;
    size_t width;
    int index_opcode;
    int index_mnemonic;
    size_t data_len;
    uint8_t data[]; //Hold the data 
} bit_elem_t;

/**
 * @brief Create the corresponding bitpattern element
 * 
 * @param type The type of the bitpattern element
 * @param width The width in bits
 * @param data The corresponding data
 * @return bit_elem_t* Return a pointer to the newly created bitpattern element
 */
bit_elem_t *bit_elem_init(BPTYPE_t type, size_t width, void *data);