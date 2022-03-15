#pragma once

#include <stdlib.h>
#include <stdint.h>

#include "dynamic_array.h"
#include "tokeniser.h"
#include "commands.h"
#include "generator.h"

// Enum of default tokens
enum
{
    eT_ARG_SEPARATOR = 0,
    eT_COMMENT,
    eT_NEWLINE,
    eT_WHITESPACE,
    eT_ADDRESS,
    eT_LABEL,
    eT_IMMEDIATE_INT,
    eT_IMMEDIATE_CHAR,
    eT_IDENTIFIER,
    eT_CONSTANT_DIR,
};

int extern token_id_lookup[];

extern darray_t* tokens;

/**
 * @brief Init the token array
 * 
 */
void lexer_init();

void lexer_generate();