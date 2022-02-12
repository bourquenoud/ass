#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

#include "macro.h"
#include "xmalloc.h"
#include "dynamic_array.h"
#include "hash_array.h"

typedef struct state_s state_t;

typedef struct
{
    int condition;
    int next_state_id;
} transistion_t;

struct state_s
{
    int id;
    int end_state;
    darray_t *transitions_ttrans;
};

typedef struct
{
    darray_t *states_tstate;
} state_machine_t;

state_t *state_machine_add_state(state_machine_t *state_machine, int end_state);
state_machine_t state_machine_init();
state_machine_t state_machine_merge(state_machine_t *state_machine_a, state_machine_t *state_machine_b);
state_t *state_machine_get_state(state_machine_t *state_machine, int state_id);
bool state_compare_states(state_t *s1, state_t *s2);
bool state_compare_transitions(transistion_t *t1, transistion_t *t2);
void state_machine_reduce(state_machine_t *state_machine);
void state_machine_replace_id(state_machine_t *state_machine, int new_id, int old_id);
transistion_t *state_machine_get_transitions(state_machine_t *state_machine, int state_id);

state_t state_init_state(int id);

int _state_add_transition(int _arg_count, state_t *state, int next_state_id, ...);
#define state_add_transition(...) _state_add_transition((PP_NARG(__VA_ARGS__)) - 2, __VA_ARGS__)

void state_machine_print(state_machine_t *state_machine, FILE *file_descriptor);
