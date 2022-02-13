
#include "state_machine.h"

#define MAX_STATE 256

static void xmalloc_callback(int err);

state_machine_t state_machine_init()
{
    const int start_id = 0;
    state_machine_t new_state_machine;
    state_t new_state;
    new_state = state_init_state(start_id);
    new_state.end_state = 0;
    new_state_machine.states_tstate = darray_init(sizeof(state_t));
    darray_add(&(new_state_machine.states_tstate), new_state);

    return new_state_machine;
}

state_t *state_machine_add_state(state_machine_t *state_machine, int end_state)
{
    // Get the dynamic array as a static array
    state_t *state_array = (state_t *)(state_machine->states_tstate->element_list);

    // Get the first available state id
    // NOTE: O(N²), but not a big problem as the state machine will remain relatively small
    //  the algorithm complexity could be improved by sorting the states by id
    int new_id;
    for (new_id = 0; new_id < MAX_STATE; new_id++)
    {
        bool available = true;
        for (int i = 0; i < state_machine->states_tstate->count; i++)
        {
            // Id is already taken
            if (state_array[i].id == new_id)
            {
                available = false;
                break;
            }
        }
        if (available)
            break;
    }

    // Create a new state with the corresponding ID and add it to the state machine
    state_t new_state = state_init_state(new_id);
    new_state.end_state = end_state;
    darray_add(&(state_machine->states_tstate), new_state);

    // Return a pointer to the last element (the one we added)
    return (state_t *)(state_machine->states_tstate->element_list) + state_machine->states_tstate->count - 1;
}

state_t *state_machine_get_state(state_machine_t *state_machine, int state_id)
{
    // Look for the state id
    state_t *state_array = (state_t *)state_machine->states_tstate->element_list;
    for (size_t i = 0; i < state_machine->states_tstate->count; i++)
    {
        if (state_array[i].id == state_id)
            return state_array + i;
    }
    return NULL;
}

transistion_t *state_machine_get_transitions(state_machine_t *state_machine, int state_id)
{
    // Look for the state id
    state_t *state_array = (state_t *)state_machine->states_tstate->element_list;
    for (size_t i = 0; i < state_machine->states_tstate->count; i++)
    {
        if (state_array[i].id == state_id)
            return (transistion_t *)(state_array[i].transitions_ttrans->element_list);
    }
    return NULL;
}

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
// TODO: Make it less terrible. Seriously the way it is done sucks
int _state_add_transition(int _arg_count, state_t *state, int next_state_id, ...)
{
    va_list args;
    va_start(args, next_state_id);

    // Check if it already exists
    for (size_t i = 0; i < _arg_count; i++)
    {
        bool exists = false;
        int cond = va_arg(args, int);
        transistion_t *transition_array = (transistion_t *)darray_get_ptr(&(state->transitions_ttrans), 0);
        for (size_t j = 0; j < state->transitions_ttrans->count; j++)
        {
            if (transition_array->next_state_id == next_state_id && transition_array->condition == cond)
            {
                exists = false;
                break;
            }
        }

        //Add the transition if it doesn't already exist
        if (!exists)
        {
            transistion_t new_transition = {.condition = cond, .next_state_id = next_state_id};
            darray_add(&(state->transitions_ttrans), new_transition);
        }
    }

    va_end(args);

    return 0;
}

state_machine_t state_machine_merge(state_machine_t *state_machine_a, state_machine_t *state_machine_b)
{
    int corresp_table[state_machine_b->states_tstate->count][2];

    state_machine_t new_state_machine = state_machine_init();

    // Copy the state machine A in one go
    darray_copy(&(new_state_machine.states_tstate), &(state_machine_a->states_tstate));

    // Add each state of the state machine B, skipping the starting state
    for (size_t i = 1; i < state_machine_b->states_tstate->count; i++)
    {
        // Copy one state
        state_t *old_state = darray_get_ptr(&(state_machine_b->states_tstate), i);
        state_t *new_state = state_machine_add_state(&new_state_machine, old_state->id);
        darray_copy(&(new_state->transitions_ttrans), &(old_state->transitions_ttrans)); // Clone the transitions

        // Add to the correspondance table. The id has likely changed so we will remap it
        //  once all state have been copied
        corresp_table[i][0] = old_state->id;
        corresp_table[i][1] = new_state->id;
    }

    // Don't remap the starting state, as it is the only one shared by both fsm
    corresp_table[0][0] = 0;
    corresp_table[0][1] = 0;

    // Remap
    // TODO: 4 level for loop ? Really ? Fix that ASAP
    for (size_t i = 0; i < new_state_machine.states_tstate->count; i++)
    {
        // Check if it is a B state machine's state
        for (size_t j = 1; j < state_machine_b->states_tstate->count; j++)
        {
            state_t *state = (state_t *)darray_get_ptr(&(new_state_machine.states_tstate), i);

            // Remap the transitions
            if (corresp_table[j][1] == (state->id))
            {
                int n_trans = state->transitions_ttrans->count;
                transistion_t *trans_array = ((transistion_t *)darray_get_ptr(&(state->transitions_ttrans), 0));
                for (size_t k = 0; k < n_trans; k++)
                {
                    int old_id = trans_array[k].next_state_id;
                    int new_id = -1;
                    for (size_t l = 0; l < state_machine_b->states_tstate->count; l++)
                    {
                        if (corresp_table[l][0] == old_id)
                            new_id = corresp_table[l][1];
                    }
                    if (new_id == -1)
                        abort();

                    trans_array[k].next_state_id = new_id;
                }

                break;
            }
        }
    }

    // Merge the start transitions of machine B with the new machine
    state_t *old_state = darray_get_ptr(&(state_machine_b->states_tstate), 0);
    state_t *new_state = darray_get_ptr(&(new_state_machine.states_tstate), 0);

    int n_trans = old_state->transitions_ttrans->count;
    transistion_t *trans_array = ((transistion_t *)darray_get_ptr(&(old_state->transitions_ttrans), 0));
    for (size_t k = 0; k < n_trans; k++)
    {
        int old_id = trans_array[k].next_state_id;
        int new_id = -1;
        for (size_t l = 0; l < state_machine_b->states_tstate->count; l++)
        {
            if (corresp_table[l][0] == old_id)
                new_id = corresp_table[l][1];
        }
        if (new_id == -1)
            abort();

        state_add_transition(new_state, new_id, trans_array[k].condition);
    }

    return new_state_machine;
}

void state_machine_reduce(state_machine_t *state_machine)
{
    // Two state can be merged if they have the same end_state and the same transitions
    bool has_merged = true;

    //
    while (has_merged)
    {
        int count = state_machine->states_tstate->count;
        state_t *state_array = (state_t *)darray_get_ptr(&(state_machine->states_tstate), 0);
        has_merged = false;
        // We check each state against all other states
        for (size_t i = 0; i < count && !has_merged; i++)
        {
            for (size_t j = i + 1; j < count && !has_merged; j++)
            {
                if (state_compare_states(state_array + i, state_array + j))
                {
                    // We can merge the two states
                    state_machine_replace_id(state_machine, state_array[i].id, state_array[j].id);
                    darray_remove_at(&(state_machine->states_tstate), 1, j);
                    has_merged = true;
                }
            }
        }
    }
}

void state_machine_replace_id(state_machine_t *state_machine, int new_id, int old_id)
{
    // For each state...
    int s_count = state_machine->states_tstate->count;
    state_t *state_array = darray_get_ptr(&(state_machine->states_tstate), 0);
    for (size_t i = 0; i < s_count; i++)
    {
        // ...and for each transition
        int t_count = state_array[i].transitions_ttrans->count;
        transistion_t *transition_array = darray_get_ptr(&(state_array[i].transitions_ttrans), 0);

        for (size_t j = 0; j < t_count; j++)
        {
            // If it points to the old id, replace with the new id
            if (transition_array[j].next_state_id == old_id)
            {
                transition_array[j].next_state_id = new_id;
            }
        }
    }
}

void state_machine_make_deterministic(state_machine_t *state_machine)
{
    darray_t *dfa_state_array = darray_init(sizeof(bitarray_t));
}

// return true if identique, false otherwise
bool state_compare_states(state_t *s1, state_t *s2)
{
    size_t count = s1->transitions_ttrans->count;
    transistion_t *s1_trans_array = darray_get_ptr(&(s1->transitions_ttrans), 0);
    transistion_t *s2_trans_array = darray_get_ptr(&(s2->transitions_ttrans), 0);

    if (s1->end_state != s2->end_state)
        return false;

    if (s1->transitions_ttrans->count != s2->transitions_ttrans->count)
        return false;

    // For each condition of t1, check if t2 contains it too
    for (size_t i = 0; i < count; i++)
    {
        bool match = false;
        for (size_t j = 0; j < count; j++)
        {
            if (state_compare_transitions(s1_trans_array + i, s2_trans_array + j))
            {
                match = true;
                break;
            }
        }
        if (!match)
            return false;
    }

    // We survived the check, all transitions have a match
    return true;
}

// return true if identique, false otherwise
bool state_compare_transitions(transistion_t *t1, transistion_t *t2)
{
    return (t1->condition == t2->condition && t1->next_state_id == t2->next_state_id);
}

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
            fprintf(file_descriptor, "  \t[label=\"(%i)\"]\n", transition[j].condition);
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
