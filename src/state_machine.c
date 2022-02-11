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
void state_machine_print(state_machine_t *state_machine, FILE *file_descriptor)
{
    // Get the dynamic array as a static array
    state_t *state_array = (state_t *)(state_machine->states_tstate->element_list);

    fputs("digraph STATE_MACHINE {\n"
          "\tnode [shape=ellipse,width=1.25]\n"
          "\tmclimit=100\n"
          "\tsplines=spline\n\n",
          file_descriptor);

    // Print all state and transitions
    for (int i = 0; i < state_machine->states_tstate->count; i++)
    {
        // Get the dynamic array as a static array
        transistion_t *transition = (transistion_t *)(state_array[i].transitions_ttrans->element_list);

        for (int j = 0; j < state_array[i].transitions_ttrans->count; j++)
        {
            fprintf(file_descriptor, "\t%i -> %i", state_array[i].id, transition[j].next_state_id);
            fputs("  \t[label=\"(", file_descriptor);
            for (size_t k = 0; k < transition[j].conditions_tint->count; k++)
            {
                fprintf(
                    file_descriptor,
                    "%i%c",
                    *((int *)(transition[j].conditions_tint->element_list) + k),
                    k == transition[j].conditions_tint->count - 1 ? ')' : ',');
            }
            fputs("\"]\n", file_descriptor);
        }
    }

    fputs("}\n", file_descriptor);
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
