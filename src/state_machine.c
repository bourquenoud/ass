#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "state_machine.h"
#include "macro.h"
#include "xmalloc.h"

state_t state_init_state(int id)
{
    state_t new_state;
    new_state.id = id;

    // Create a dynamic array of transitions
    new_state.transitions_ttrans = darray_init(sizeof(transistion_t));

    return new_state;
}

// NOTE: This function is wrapped inside a macro, which passes the number of arguments
//  as "_arg_count". When calling "state_add_transition" (the wrapper), the number of
//  arguments should not be given, as it is computed by the preprocessor.
void _state_add_transition(int _arg_count, state_t *state, int next_state_id, ...)
{
    va_list args;
    va_start(args, next_state_id);

    transistion_t new_transition;
    new_transition.next_state_id = next_state_id;

    // Create a dynamic array of int
    new_transition.conditions_tint = darray_init(sizeof(int));

    // We add all the conditions to the transition, without checking for conflicts
    for (int i = 0; i < _arg_count; i++)
    {
        int condition = va_arg(args, int);
        darray_add(new_transition.conditions_tint, condition);
    }

    //Add the new transition the the state
    darray_add(state->transitions_ttrans, new_transition);
}

/********************************************************/

void xmalloc_callback(int err)
{
    fputs("\033[31mError in " STR(__FILE__) " : ", stderr);
    if (0 == err)
        fputs("Cannot allocate zero length memory\033[0m\n", stderr);
    else if (1 == err)
        fputs("Malloc returned a NULL pointer\033[0m\n", stderr);
    else
        fputs("Unknown errro\033[0m\n", stderr);
}
