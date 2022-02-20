#include "parser.h"

#include "failure.h"

state_machine_t parser_arrays_to_nfa(int count, const rule_def_t **rules)
{
    state_machine_t new_state_machine;
    state_machine_t merged_state_machine;

    // Generate the first fsm
    merged_state_machine = parser_rule_to_nfa(rules[0]);

    for (size_t i = 1; i < count; i++)
    {
        // Generate a new state machine and reduce if possible
        new_state_machine = parser_rule_to_nfa(rules[i]);
        merged_state_machine = state_machine_merge(&merged_state_machine, &new_state_machine);
        state_machine_reduce(&merged_state_machine);
    }

    return merged_state_machine;
}

state_machine_t parser_rule_to_nfa(const rule_def_t *rule)
{
    if (rule->count == 0)
    {
        fail_error("Empty rule");
        exit(EXIT_FAILURE);
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

    for (size_t i = 0; i < rule->count; i++)
    {
        last_state = current_state;

        // Generate a new state and add all the characters in the set
        current_state = state_machine_add_state(&new_state_machine, false);
        state_add_transition(last_state, current_state->id, rule->tokens[i]);
    }
    current_state->end_state = true;
    current_state->output = rule->id;

    return new_state_machine;
}