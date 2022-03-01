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

state_machine_t tokeniser_array_to_nfa(int count, const token_def_t *tokens_array);
state_machine_t tokeniser_token_to_nfa(const token_def_t token);
