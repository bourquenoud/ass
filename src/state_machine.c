
#include "state_machine.h"
#include "bitarray.h"

#include "failure.h"

#define MAX_STATE 1024

static void xmalloc_callback(int err);

state_machine_t state_machine_init()
{
    const int start_id = 0;
    state_machine_t new_state_machine;
    state_t new_state;
    new_state = state_init_state(start_id);
    new_state.end_state = 0;
    new_state.output = -1;
    new_state_machine.states_tstate = darray_init(sizeof(state_t));
    darray_add(&(new_state_machine.states_tstate), new_state);

    return new_state_machine;
}

void state_machine_remove_state(state_machine_t *state_machine)
{
    darray_remove(&(state_machine->states_tstate), 1);
}

state_t *state_machine_add_state(state_machine_t *state_machine, int end_state)
{
    // Get the dynamic array as a static array
    state_t *state_array = (state_t *)(state_machine->states_tstate->element_list);

    // Get the first available state id
    // NOTE: O(N²), but not a big problem as the state machine will remain relatively small
    // TODO: Optimise
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
    new_state.output = -1;

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

        // Add the transition if it doesn't already exist
        if (!exists)
        {
            transistion_t new_transition = {.condition = cond, .next_state_id = next_state_id};
            darray_add(&(state->transitions_ttrans), new_transition);
        }
    }

    va_end(args);

    return 0;
}

state_t *state_machine_get_by_id(state_machine_t *state_machine, int id)
{
    for (size_t i = 0; i < state_machine->states_tstate->count; i++)
    {
        state_t *current_state = darray_get_ptr(&(state_machine->states_tstate), i);
        if (current_state->id == id)
            return current_state;
    }
    return NULL;
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
        new_state->end_state = old_state->end_state;
        new_state->output = old_state->output;

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

    // Mark as end state if needed
    if (!new_state->end_state && old_state->end_state)
    {
        new_state->output = old_state->output;
        new_state->end_state = true;
    }
    else if (new_state->end_state && old_state->end_state && old_state->output != new_state->output)
    {
        fail_error("Conflicting empty tokens");
        exit(EXIT_FAILURE);
    }

    return new_state_machine;
}

void state_machine_reduce(state_machine_t *state_machine)
{
    // Two state can be merged if they have the same end_state, the same output and the same transitions
    bool has_merged = true;

    // Stop when we can not merge more states
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

state_machine_t state_machine_make_deterministic(state_machine_t *nfa)
{
    // Contains all the state of the old state machine. It is
    //  organised by index. This means that if, for example,
    //  the 5th bit and the 11th bit are set the it is an union of
    //  the 5th state and the 11th state, no matter their id
    int nfa_state_count = nfa->states_tstate->count;
    state_t *nfa_state_array = darray_get_ptr(&(nfa->states_tstate), 0);

    // Used to keep track of output values
    int last_id = -1;
    int output = -1;

    // Contains the current state combination
    bitarray_t current_state_combination;
    bitarray_set_all(&current_state_combination, false);

    // Contains the generated state combination
    bitarray_t new_state_combination;

    // Store all generated state in the form of a bit array
    darray_t *generated_state_table = darray_init(sizeof(bitarray_t));

    // Store all states where the DFA can be at the moment
    darray_t *current_states = darray_init(sizeof(state_t));

    // Store all conditions of the current states
    darray_t *current_transitions = darray_init(sizeof(transistion_t));

    // The new deterministic state machine
    state_machine_t dfa = state_machine_init();
    state_t *dfa_current_state = (state_t *)darray_get_ptr(&(dfa.states_tstate), 0);

    // Begin by processing the start state
    bitarray_set(&current_state_combination, 0, true);
    darray_add(&generated_state_table, current_state_combination);

    // Generate all states until there are no more new state
    for (size_t i = 0; i < generated_state_table->count; i++)
    {
        last_id = -1;
        output = -1;

        // Get the state to process in the table
        darray_get(&generated_state_table, &current_state_combination, i);

        // Generate a new state unless we are processing the starting state
        if (i != 0)
            dfa_current_state = state_machine_add_state(&dfa, false); // End state are determined at the end

        // Make sure it is the correct id
        if (dfa_current_state->id != i)
        {
            fail_error("The id should be equal to the index. Got %i, expected %i", dfa_current_state->id, generated_state_table->count - 1);
            abort();
        }

        // Empty the current states array
        darray_empty(&current_states);
        darray_empty(&current_transitions);

        // Add all the state we are currently using
        for (size_t j = 0; j < MAX_STATE / sizeof(uint32_t); j++)
        {
            if (bitarray_get(&current_state_combination, j))
            {
                darray_add(&current_states, nfa_state_array[j]);
                if (nfa_state_array[j].output != -1)
                {
                    // Priority to the lowest id
                    if (output != -1 && nfa_state_array[j].output != output)
                    {
                        last_id, nfa_state_array[j].id;

                        // TODO: show token name
                        fail_detail("Tokens (ID %i) and (ID %i) conflict. Priority given to the lowest ID.",
                                    output,
                                    nfa_state_array[j].output);
                        output = (output < nfa_state_array[j].output) ? output : nfa_state_array[j].output;
                    }
                    else
                    {
                        output = nfa_state_array[j].output;
                        last_id = nfa_state_array[j].id;
                    }
                }

                // And add all the transitions
                for (size_t k = 0; k < nfa_state_array[j].transitions_ttrans->count; k++)
                {
                    darray_add(&current_transitions, ((transistion_t *)(nfa_state_array[j].transitions_ttrans->element_list))[k]);
                }
            }
        }

        // Set the output
        dfa_current_state->output = output;

        transistion_t *current_transition_array = (transistion_t *)darray_get_ptr(&current_transitions, 0);
        // Get all conditions to process
        for (size_t j = 0; j < current_transitions->count; j++)
        {
            // Reset the combination array
            bitarray_set_all(&new_state_combination, false); // Reset the old

            // The condition we want to add
            int cond = current_transition_array[j].condition;

            // Check if the current state already contains the condition
            bool contains_condition = false;
            int dfa_current_transitions_count = dfa_current_state->transitions_ttrans->count;
            transistion_t *dfa_current_transitions = darray_get_ptr(&(dfa_current_state->transitions_ttrans), 0);
            for (size_t k = 0; k < dfa_current_transitions_count; k++)
            {
                if (dfa_current_transitions[k].condition == cond)
                {
                    contains_condition = true;
                    break;
                }
            }

            // Check for the next condition if this one has already been computed
            if (contains_condition)
                continue;

            // Add a new transition the dfa, the id will be allocated later
            transistion_t new_transition = {.condition = cond, .next_state_id = -1};

            // Search for transitions with the same conditions in the current states we are processing
            //  and merge them
            for (size_t k = j; k < current_transitions->count; k++)
            {
                if (current_transition_array[k].condition == cond)
                {
                    // Extract the index
                    state_t *state_ptr = state_machine_get_by_id(nfa, current_transition_array[k].next_state_id);
                    int index = state_ptr - (state_t *)darray_get_ptr(&(nfa->states_tstate), 0);
                    index / sizeof(state_t);
                    bitarray_set(&new_state_combination, index, true);
                }
            }

            // Look in the table if the state already exists, and assign its id to the transition
            bitarray_t *generated_state_table_array = (bitarray_t *)darray_get_ptr(&generated_state_table, 0);
            for (size_t k = 0; k < generated_state_table->count; k++)
            {
                if (bitarray_compare(&(generated_state_table_array[k]), &new_state_combination))
                {
                    new_transition.next_state_id = k;
                    break;
                }
            }

            // If it does not already exist the we add it to the table
            if (new_transition.next_state_id < 0)
            {
                new_transition.next_state_id = generated_state_table->count;
                darray_add(&generated_state_table, new_state_combination);
            }

            // Update the id
            darray_add(&(dfa_current_state->transitions_ttrans), new_transition);
        }
    }

    // Get all end states
    darray_t *end_state_ids = darray_init(sizeof(int));
    for (size_t i = 0; i < nfa_state_count; i++)
    {
        if (nfa_state_array[i].end_state)
            darray_add(&end_state_ids, nfa_state_array[i].id);
    }

    int *end_state_ids_array = darray_get_ptr(&(end_state_ids), 0);
    // Mark any set containing an end state as an end state
    for (size_t i = 0; i < generated_state_table->count; i++)
    {
        last_id = -1;
        output = -1;
        for (size_t j = 0; j < end_state_ids->count; j++)
        {
            // If multiple end state with different output conflict, we get a rule conflict
            if (bitarray_get((bitarray_t *)darray_get_ptr(&generated_state_table, i), end_state_ids_array[j]))
            {
                state_t *end_state = ((state_t *)darray_get_ptr(&(dfa.states_tstate), i));
                if (output > 0 && end_state->output != output)
                {
                    fprintf(stderr, "Rule %i conflicts with rule %i", last_id, end_state->id);
                    exit(EXIT_FAILURE);
                }
                end_state->end_state = true;
            }
        }
    }

#ifdef DEBUG
    fputs("Generated array :\n\t", stdout);
    for (size_t i = 0; i < generated_state_table->count; i++)
    {
        fprintf(stdout, "(%4i) ", i);
        for (size_t j = 0; j < MAX_STATE; j++)
        {
            if (bitarray_get((bitarray_t *)darray_get_ptr(&generated_state_table, i), j))
            {
                putc('1', stdout);
            }
            else
            {
                putc('0', stdout);
            }
            if (j % 4 == 3 && j != MAX_STATE - 1)
                putc('\'', stdout);
        }
        fputs(";\n\t", stdout);
    }
#endif // DEBUG

    // Reduction pass
    state_machine_reduce(&dfa);

    return dfa;
}

// return true if identique, false otherwise
bool state_compare_states(state_t *s1, state_t *s2)
{
    size_t count = s1->transitions_ttrans->count;
    transistion_t *s1_trans_array = darray_get_ptr(&(s1->transitions_ttrans), 0);
    transistion_t *s2_trans_array = darray_get_ptr(&(s2->transitions_ttrans), 0);

    if (s1->end_state != s2->end_state)
        return false;

    if (s1->output != s2->output)
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
          "\tnode [shape=circle,width=1]\n"
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

        if (state_array[i].end_state)
        {
            fprintf(file_descriptor, "\t%i \t[shape=doublecircle]\n", state_array[i].id);
        }
    }

    fputs("}\n", file_descriptor);
}

void char_to_escape_seq(char c, char *buff);

void state_machine_print_char(state_machine_t *state_machine, FILE *file_descriptor)
{
    // Get the dynamic array as a static array
    state_t *state_array = (state_t *)(state_machine->states_tstate->element_list);

    fputs("digraph STATE_MACHINE {\n"
          "\tnode [shape=circle,width=1]\n"
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
            char buff[6];
            char_to_escape_seq((char)(transition[j].condition), buff);
            fprintf(file_descriptor, "\t%i -> %i", state_array[i].id, transition[j].next_state_id);
            fprintf(file_descriptor, "  \t[label=\"(%s)\"]\n", buff);
        }

        if (state_array[i].end_state)
        {
            fprintf(file_descriptor, "\t%i \t[shape=doublecircle, label=\"%i\\n>%i<\"]\n", state_array[i].id, state_array[i].id, state_array[i].output);
        }
    }

    fputs("}\n", file_descriptor);
}

void char_to_escape_seq(char c, char *buff)
{
    if (c >= '!' && c <= '~')
    {
        buff[0] = c;
        buff[1] = '\0';
    }
    else if (c == '\n')
    {
        buff[0] = '\\';
        buff[1] = '\\';
        buff[2] = 'n';
        buff[3] = '\0';
    }
    else if (c == '\r')
    {
        buff[0] = '\\';
        buff[1] = '\\';
        buff[2] = 'r';
        buff[3] = '\0';
    }
    else if (c == '\n')
    {
        buff[0] = '\\';
        buff[1] = '\\';
        buff[2] = 't';
        buff[3] = '\0';
    }
    else if (c == '"')
    {
        buff[0] = '\\';
        buff[1] = '\\';
        buff[2] = '"';
        buff[3] = '\0';
    }
    else
    {
        buff[0] = '\\';
        buff[1] = '\\';
        buff[2] = 'x';
        buff[3] = ((c >> 4) & 0x0F) < 10 ? ((c >> 4) & 0x0F) + '0' : ((c >> 4) & 0x0F) - 10 + 'A';
        buff[4] = (c & 0x0F) < 10 ? (c & 0x0F) + '0' : (c & 0x0F) - 10 + 'A';
        buff[5] = '\0';
    }
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
