#pragma once

#include "state_machine.h"
#include "pattern_compiler.h"

extern darray_t *rule_list_tint;

typedef struct
{
    int id;
    char *name;
    char* action;
    int count;
    int tokens[];
} rule_def_t;

/**
 * @brief Generate a non-deteministic finite state machine from the list of rules
 * 
 * @param count The number of rules
 * @param rules The list of rules
 * @return state_machine_t The generated finite state machine
 */
state_machine_t parser_arrays_to_nfa(int count, const rule_def_t **rules);

/**
 * @brief Generate a non-deterministric finite state machine from a single rule
 * 
 * @param rule The rule to generate the finite state machine from
 * @return state_machine_t The generated finite state machine
 */
state_machine_t parser_rule_to_nfa(const rule_def_t *rule);