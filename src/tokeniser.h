#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include "state_machine.h"
#include "linked_list.h"
#include "pattern_compiler.h"

typedef struct
{
    int id;
    char *name;
    char *pattern;
    char *action;
    void *data;
} token_def_t;

/**
 * @brief Generate an nfa from an array of token definitions
 * 
 * @param count The number of tokens definitions
 * @param tokens_array The array of tokens definitions
 * @return state_machine_t The nfa
 */
state_machine_t tokeniser_array_to_nfa(int count, const token_def_t *tokens_array);

/**
 * @brief Generate an nfa from a token definition
 * 
 * @param token The token definition
 * @return state_machine_t The nfa
 */
state_machine_t tokeniser_token_to_nfa(const token_def_t token);
