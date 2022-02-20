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
    return pattern_compiler(rule->count, rule->tokens, rule->id);
}