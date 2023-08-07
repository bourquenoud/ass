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
    eT_IMMEDIATE_HEX,
    eT_IMMEDIATE_DEC,
    eT_IMMEDIATE_OCT,
    eT_IMMEDIATE_BIN,
    eT_IMMEDIATE_CHAR,
    eT_IDENTIFIER,
    eT_CONSTANT_DIR,
    eT_MACRO_DIR,
    eT_END_MACRO_DIR,
};

int extern token_id_lookup[];

extern darray_t *tokens;

/**
 * @brief Init the token array
 *
 */
void lexer_init();

/**
 * @brief Generate the the lexer from the list of tokens
 *
 */
void lexer_generate();