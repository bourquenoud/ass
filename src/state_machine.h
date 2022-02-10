#pragma once

#include "dynamic_array.h"

typedef struct state_s state_t;

typedef struct
{
    int next_state_id;
    darray_t* conditions_tint;
}transistion_t;

struct state_s
{
    int id;
    darray_t* transitions_ttrans;
};

state_t state_init_state(int id);
void state_add_state(state_t *state_list, int id);

void _state_add_transition(int _arg_count, state_t *state, int next_state_id, ...);
#define state_add_transition(...) _state_add_transition((PP_NARG(__VA_ARGS__)) - 1, __VA_ARGS__)