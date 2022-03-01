#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

#include "parser.h"
#include "state_machine.h"
#include "hash_array.h"
#include "dynamic_array.h"
#include "tokeniser.h"
#include "parser.h"
#include "macro.h"
#include "xmalloc.h"
#include "commands.h"
#include "bitpattern.h"
#include "ast_node.h"

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

/**
 * @brief Generate the parser from a list of rules
 * 
 * @details If different rules have the same id they are merged together as
 *          a single rule without creating any error or conflict. Lower id
 *          have priority over higher ids.
 *
 * @param count The number of rules
 * @param _rules The rule array
 */
void generator_generate_parser(int count, const rule_def_t **_rules);

/**
 * @brief Generate the token action for a specific enum pattern
 * 
 * @param pattern Pointer to the reference pattern
 * @return char* Return a string of C code
 */
char *generator_generate_pattern_action(pattern_t* pattern);

/**
 * @brief Generate the rule action for a specific opcode
 * 
 * @param opcode Reference opcode to use
 * @return char* Return a string of C code
 */
char *generator_generate_opcode_action(opcode_t opcode);

/**************************************************/
/*                   CALLBACKS                    */
/**************************************************/
void generator_data_union(int indent);
void generator_lexer_actions(int indent);
void generator_lexer_action_list(int indent);
void generator_lexer_switch(int indent);
void generator_parser_actions(int indent);
void generator_parser_action_list(int indent);
void generator_parser_switch(int indent);
void generator_token_enum(int indent);
void generator_token_names(int indent);
