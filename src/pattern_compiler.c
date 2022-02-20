#include "pattern_compiler.h"

#include "failure.h"

// Generate a state machine matching the provided sequence
state_machine_t pattern_compiler(size_t count, const int *sequence, int output)
{

    state_machine_t new_state_machine = state_machine_init();

    state_t *current_state = darray_get_ptr(&(new_state_machine.states_tstate), 0); // State to be processed
    state_t *last_state = NULL;                                                     // Last processed state
    state_t *save_state = NULL;                                                     // Saved state for state skipping

    // The set of all conditions
    darray_t *set = darray_init(sizeof(int));

    // Mark the previous token as optional
    bool optional = false;

    // Used for the set parsing
    bool parsing_set = false; // If we are processing a set

    // TODO: implement negative sets
    bool negative_set = false; // If the set we are processing is negative

    for (size_t i = 0; i < count; i++)
    {
        // Empty the set unless we are currently processing a set
        if (!parsing_set)
            darray_empty(&set);

        // Negative values are control characters, except -1 which is EOF
        if (sequence[i] < -1)
        {
            // Execute the command
            switch (-sequence[i])
            {
            case '+': // Make the current state loop on itself
                if (parsing_set)
                { // Invalid in a set
                    fail_error("Unexpected '+' control in a set, at position %i", i);
                    exit(EXIT_FAILURE);
                }

                for (size_t j = 0; j < last_state->transitions_ttrans->count; j++)
                {
                    transistion_t *trans = darray_get_ptr(&(last_state->transitions_ttrans), j);
                    state_add_transition(current_state, current_state->id, trans->condition);
                }
                continue;

            case '*': // Remove the new state and make the old one loop on itself
                if (parsing_set)
                { // Invalid in a set
                    fail_error("Unexpected '+' control in a set, at position %i", i);
                    exit(EXIT_FAILURE);
                }

                state_machine_remove_state(&new_state_machine);
                current_state = last_state;
                for (size_t j = 0; j < last_state->transitions_ttrans->count; j++)
                {
                    transistion_t *trans = darray_get_ptr(&(last_state->transitions_ttrans), j);
                    trans->next_state_id = last_state->id;
                }
                continue;

            case '?': // Save the last state to make it skip over the current one after
                if (parsing_set)
                { // Invalid in a set
                    fail_error("Unexpected '?' control in a set, at position %i", i);
                    exit(EXIT_FAILURE);
                }

                optional = true;
                save_state = last_state;
                continue;

            case '[': // Enter a set
                if (parsing_set)
                { // Invalid in a set
                    fail_error("Unexpected '[' control in a set, at position %i", i);
                    exit(EXIT_FAILURE);
                }

                if (sequence[i + 1] == '^')
                {
                    i++;
                    negative_set = true;
                    parsing_set = true;
                }
                else
                {
                    negative_set = false;
                    parsing_set = true;
                }
                break;

            case ']': // Exit the set
                if (!parsing_set)
                { // Invalid outside of a set
                    fail_error("Unexpected ']' control outside of a set, at position %i", i);
                    exit(EXIT_FAILURE);
                }

                parsing_set = false;
                break;

            case '-': // Range value in set
                if (!parsing_set)
                {
                    // Invalid outside of a set
                    fail_error("Unexpected '-' control outside of a set, at position %i", i);
                    exit(EXIT_FAILURE);
                }

                int start_range = *((int *)(darray_get_ptr(&set, set->count - 1)));
                int end_range;
                i++;

                // Check for a valid range
                if (i > count || sequence[i] < -1)
                {
                    // Invalid outside of a set
                    fail_error("Invalid range, at position %i", i - 1);
                    exit(EXIT_FAILURE);
                }

                // Swap the direction if necessary
                if (start_range > sequence[i])
                {
                    // Swap and skip end character as it as already been added
                    end_range = start_range - 1;
                    start_range = sequence[i];
                }
                else
                {
                    // Don't swap, but skip the first character as it as already been added
                    start_range++;
                    end_range = sequence[i];
                }

                // Add all character from start to current char
                for (char j = start_range; j <= end_range; j++)
                {
                    darray_add(&set, j);
                }
                break;

            // TODO: implement group and or
            case '(': // Match group not implemented yet
            case ')': // Match group not implemented yet
            case '|': // Or not implemented yet
                fail_error("Unimplemented feature. Please refer to the documentation.");
                exit(EXIT_FAILURE);
                break;

            default: // Shouldn't go there
                fail_error("Unkown control character.");
                exit(EXIT_FAILURE);
                break;
            }
            // Skip the state generation and don't add the character
            continue;
        }

        // Add the character to the set if it is not a control character
        darray_add(&set, sequence[i]);

        // Skip the state generation until we exit the set
        if (parsing_set)
            continue;

        // Save the last state for later linking
        last_state = current_state;

        // Generate a new state and add all the characters in the set
        current_state = state_machine_add_state(&new_state_machine, false);
        for (int i = 0; i < set->count; i++)
        {
            state_add_transition(last_state, current_state->id, *((int *)darray_get_ptr(&set, i)));
        }

        // Create a transition that skips over the last state if it was optional
        if (optional)
        {
            for (size_t i = 0; i < last_state->transitions_ttrans->count; i++)
            {
                transistion_t *trans = darray_get_ptr(&(last_state->transitions_ttrans), i);
                state_add_transition(save_state, current_state->id, trans->condition);
            }
            optional = false;
        }
    }

    // Mark the last state as an end state
    current_state->end_state = true;
    current_state->output = output;

    // Last char was optional, we mark the previous state as optional
    if (optional)
    {
        last_state->end_state = true;
        current_state->output = output;
    }

    return new_state_machine;
}