#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

#include "state_machine.h"
#include "hash_array.h"
#include "dynamic_array.h"
#include "tokeniser.h"
#include "macro.h"
#include "xmalloc.h"

/**
 * @brief Set the file descriptor for the generator
 *
 * @param file_descriptor The file descriptor to which the generator will write
 */
void generator_set_file_descriptor(FILE *file_descriptor);

/**
 * @brief Generate the lexer from a list of tokens
 *
 * @param count The number of token
 * @param _tokens The token array
 */
void generator_generate_lexer(int count, const token_def_t *_tokens);

/**************************************************/
/*                   CALLBACKS                    */
/**************************************************/
void generator_lexer_switch(int indent);
void generator_token_enum(int indent);
void generator_token_names(int indent);
