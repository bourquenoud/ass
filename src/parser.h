#pragma once

#include "state_machine.h"

typedef struct
{
    int id;
    char *name;
    char* action;
    int count;
    int tokens[];
} rule_def_t;

state_machine_t parser_arrays_to_nfa(int count, const rule_def_t **rules);
state_machine_t parser_rule_to_nfa(const rule_def_t *rule);