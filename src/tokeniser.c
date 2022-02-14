#include "tokeniser.h"

#include "failure.h"

state_machine_t tokeniser_array_to_nfa(int count, const char const **string_array)
{
    state_machine_t new_state_machine;
    state_machine_t merged_state_machine;

    // Generate the first fsm
    merged_state_machine = tokeniser_str_to_nfa(string_array[0]);

    for (size_t i = 1; i < count; i++)
    {
        // Generate a new state machine and reduce if possible
        new_state_machine = tokeniser_str_to_nfa(string_array[i]);
        merged_state_machine = state_machine_merge(&merged_state_machine, &new_state_machine);
        state_machine_reduce(&merged_state_machine);
    }

    return merged_state_machine;
}

// Generate a state machine matching the provided string
state_machine_t tokeniser_str_to_nfa(const char *string)
{
    // Get the string lenght and scan for illegal characters
    char const *c = string;
    size_t len = 0;
    while (*c != '\0')
    {
        if (*c <= 0x1F || *c >= 0x7F)
        {
            fail_error("Illegal character in pattern '%s'", string);
            exit(EXIT_FAILURE);
        }
        len++;
        c++;
    }

    state_machine_t new_state_machine = state_machine_init();

    state_t *current_state = darray_get_ptr(&(new_state_machine.states_tstate), 0); // State to be processed
    state_t *last_state = NULL;                                                     // Last processed state
    state_t *end_state = NULL;                                                      // State pointed by this pointer at the end is marked as final state

    for (size_t i = 0; i < len; i++)
    {
        last_state = current_state;

        // Generate a new state for each character
        current_state = state_machine_add_state(&new_state_machine, false);
        state_add_transition(last_state, current_state->id, string[i]);
    }
    end_state = current_state;
    end_state->end_state = true;

    return new_state_machine;
}