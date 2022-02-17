#include "tokeniser.h"

#include "failure.h"

state_machine_t tokeniser_array_to_nfa(int count, const token_def_t *tokens)
{
    state_machine_t new_state_machine;
    state_machine_t merged_state_machine;

    // Generate the first fsm
    merged_state_machine = tokeniser_token_to_nfa(tokens[0]);

    for (size_t i = 1; i < count; i++)
    {
        // Generate a new state machine and reduce if possible
        new_state_machine = tokeniser_token_to_nfa(tokens[i]);
        merged_state_machine = state_machine_merge(&merged_state_machine, &new_state_machine);
        state_machine_reduce(&merged_state_machine);
    }

    return merged_state_machine;
}

// Generate a state machine matching the provided string
state_machine_t tokeniser_token_to_nfa(const token_def_t token)
{
    // Get the string lenght and scan for illegal characters
    char *string = token.pattern;
    char *c = string;
    size_t len = 0;
    while (*c != '\0')
    {
        if (*c <= 0x1F || *c >= 0x7F)
        {
            fail_error("Illegal character in pattern '%s' (%#2x)", string, *c);
            exit(EXIT_FAILURE);
        }
        len++;
        c++;
    }

    state_machine_t new_state_machine = state_machine_init();

    state_t *current_state = darray_get_ptr(&(new_state_machine.states_tstate), 0); // State to be processed
    state_t *last_state = NULL;                                                     // Last processed state
    state_t *end_state = NULL;                                                      // State pointed by this pointer at the end is marked as final state

    state_t *tmp_state = NULL;

    // The set of all conditions
    darray_t *set = darray_init(sizeof(char));

    bool escaped = false;
    bool optional = false;

    // Used for the set parsing
    bool parsing_set = false;  // If we are processing a set
    bool negative_set = false; // If the set we are processing is negative

    char processed_char;

    for (size_t i = 0; i < len; i++)
    {

        if (!parsing_set)
            darray_empty(&set);

        if (escaped)
        {
            if (string[i] == 'a')
            {
                processed_char = '\a';
            }
            else if (string[i] == 'b')
            {
                processed_char = '\b';
            }
            else if (string[i] == 't')
            {
                processed_char = '\t';
            }
            else if (string[i] == 'n')
            {
                processed_char = '\n';
            }
            else if (string[i] == 'v')
            {
                processed_char = '\v';
            }
            else if (string[i] == 'f')
            {
                processed_char = '\f';
            }
            else if (string[i] == 'r')
            {
                processed_char = '\r';
            }
            else if (string[i] == 'x') // Enter hex escape sequence
            {
                char *new_ptr;
                processed_char = strtoul(string + i + 1, &new_ptr, 16);
                i = (int)((ptrdiff_t)new_ptr - (ptrdiff_t)string) - 1;
            }
            else if (string[i] >= '0' && string[i] <= '3') // Enter octal escape sequence
            {
                processed_char = (string[i++] << 6) & 0300;
                processed_char |= (string[i++] << 3) & 0070;
                processed_char |= (string[i]) & 0007;
            }
            else if (string[i] == '[')
            {
                processed_char = '[';
            }
            else if (string[i] == ']')
            {
                processed_char = ']';
            }
            else if (string[i] == '(')
            {
                processed_char = '(';
            }
            else if (string[i] == ')')
            {
                processed_char = ')';
            }
            else if (string[i] == '-' && parsing_set)
            {
                processed_char = '-';
            }
            else
            {
                fail_error("Invalid escape sequence. (\\%c)", string[i]);
            }
            darray_add(&set, processed_char);
            escaped = false;
        }
        else
        {
            if (string[i] == '\\')
            {
                escaped = true;
                continue;
            }
            else if (string[i] == '+' && !parsing_set)
            {
                for (size_t i = 0; i < last_state->transitions_ttrans->count; i++)
                {
                    transistion_t *trans = darray_get_ptr(&(last_state->transitions_ttrans), i);
                    state_add_transition(current_state, current_state->id, trans->condition);
                }
                continue;
            }
            else if (string[i] == '*' && !parsing_set)
            {
                // Remove the new state and make the old one loop on itself
                state_machine_remove_state(&new_state_machine);
                current_state = last_state;
                for (size_t i = 0; i < last_state->transitions_ttrans->count; i++)
                {
                    transistion_t *trans = darray_get_ptr(&(last_state->transitions_ttrans), i);
                    trans->next_state_id = last_state->id;
                }
                continue;
            }
            else if (string[i] == '?' && !parsing_set)
            {
                // Remove the new state and make the old one loop on itself
                optional = true;
                tmp_state = last_state;
                continue;
            }
            else if (string[i] == '[' && !parsing_set)
            {
                // Enter a set
                if (string[i + 1] == '^')
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
            }
            else if (string[i] == ']' && parsing_set)
            {
                parsing_set = false;
            }
            else if (string[i] == '-' && parsing_set)
            {
                char start_range = *((char *)(darray_get_ptr(&set, set->count - 1)));
                i++;
                for (char j = start_range + 1; j <= string[i]; j++)
                {
                    darray_add(&set, j);
                }
            }
            else
            {
                darray_add(&set, string[i]);
            }
        }

        if (parsing_set)
            continue;

        last_state = current_state;

        // Generate a new state and add all the characters in the set
        current_state = state_machine_add_state(&new_state_machine, false);
        for (int i = 0; i < set->count; i++)
        {
            state_add_transition(last_state, current_state->id, *((char *)darray_get_ptr(&set, i)));
        }

        // Create a transition that skips over the last state if it was optional
        if (optional)
        {
            for (size_t i = 0; i < last_state->transitions_ttrans->count; i++)
            {
                transistion_t *trans = darray_get_ptr(&(last_state->transitions_ttrans), i);
                state_add_transition(tmp_state, current_state->id, trans->condition);
            }
            optional = false;
        }
    }
    current_state->end_state = true;
    current_state->output = token.id;

    // Last char was optional
    if (optional)
    {
        last_state->end_state = true;
        current_state->output = token.id;
    }

    return new_state_machine;
}