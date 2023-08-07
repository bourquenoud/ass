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
    if (fail_get_verbose() >= 4)
    {
        darray_t *message = darray_init(1);
        bprintf(&message, "Generating rule %i containing %i tokens. Pattern :", rule->id, rule->count);
        for (size_t i = 0; i < rule->count; i++)
        {
            int token = rule->tokens[i];
            if (token < 0)
                bprintf(&message, "%c", -token);
            else
                bprintf(&message, "{%i}", token);
        }
        fail_debug(message->element_list);
        darray_free(&message);
    }

    return pattern_compiler(rule->count, rule->tokens, rule->id);
}