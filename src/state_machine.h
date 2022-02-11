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
    int next_state_id;
    darray_t *conditions_tint;
} transistion_t;

struct state_s
{
    int id;
    int output;
    darray_t *transitions_ttrans;
};

state_t state_init_state(int id);
void state_machine_print(state_machine_t *state_machine, FILE *file_descriptor);
