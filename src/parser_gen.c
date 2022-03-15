#include "parser_gen.h"

#include "failure.h"

static char *name_from_pattern(const char *str);

// Default rule actions
char *action_label =
    "    ASS_symbol_t new_symbol;\n"
    "    char* str = ASS_parser_stack_pop().sVal;\n"
    "    int len = strlen(str);\n"
    "    new_symbol.name = malloc(len);\n"
    "    strncpy(new_symbol.name, str, len);\n"
    "    new_symbol.name[len - 1] = '\\0';\n"
    "    new_symbol.value = ASS_current_address;\n"
    "    ASS_symbol_stack_push(new_symbol);\n"
    "    return (ASS_data_t){0, (uint64_t)0};";

char *action_address =
    "    ASS_current_address = (int)ASS_parser_stack_pop().uVal;\n"
    "    return (ASS_data_t){0, (uint64_t)0};";

char *action_opcode_DEBUG =
    "    ASS_opcode_t new_opcode;\n"
    "    new_opcode.address = ASS_current_address++;\n"
    "    new_opcode.data = 0;\n"
    "    ASS_binary_stack_push(new_opcode);\n"
    "    return (ASS_data_t){0, (uint64_t)0};";

darray_t *rule_list_tint;

void parser_init()
{
    rule_list_tint = darray_init(sizeof(int));
}

void parser_generate()
{
    int n_rules = opcode_array->count + 5;

    // Generate the rules for the parser
    int opcode_count = opcode_array->count;
    opcode_t *opcodes = darray_get_ptr(&opcode_array, 0);
    const rule_def_t **rules = xmalloc(sizeof(rule_def_t) * n_rules);
    // Generate one rule per opcode
    for (size_t i = 0; i < opcode_count; i++)
    {
        // Clear the list of token
        darray_empty(&rule_list_tint);

        // Add the token for the mnemonic
        darray_add(&rule_list_tint, opcodes[i].token_id);

        // Generate the parameter order
        bool found = true;
        for (size_t j = 0; found; j++)
        {
            linked_list_t *current = opcodes[i].bit_pattern;
            while (current != NULL)
            {
                bit_elem_t *bit_elem = (bit_elem_t *)current->user_data;
                if (bit_elem->index_mnemonic == j)
                {
                    int token_id;
                    switch (bit_elem->type)
                    {
                    case eBP_ID:
                        darray_add(&rule_list_tint, token_id_lookup[eT_IDENTIFIER]);
                        break;
                    case eBP_IMMEDIATE:
                        // Open a set
                        token_id = -(int)'[';
                        darray_add(&rule_list_tint, token_id);
                        darray_add(&rule_list_tint, token_id_lookup[eT_IMMEDIATE_INT]);
                        darray_add(&rule_list_tint, token_id_lookup[eT_IMMEDIATE_CHAR]);
                        // Close the set
                        token_id = -(int)']';
                        darray_add(&rule_list_tint, token_id);
                        break;
                    case eBP_LABEL_ABS:
                        darray_add(&rule_list_tint, token_id_lookup[eT_IDENTIFIER]); // TODO: also accept absolute adresses
                        break;
                    case eBP_LABEL_REL:
                        darray_add(&rule_list_tint, token_id_lookup[eT_IDENTIFIER]); // TODO: also accept absolute adresses
                        break;
                    case eBP_ENUM:
                        // Open a set
                        token_id = -(int)'[';
                        darray_add(&rule_list_tint, token_id);
                        // TODO: store a ptr to the enum in the bit_elem instead of a copy
                        enumeration_t *enumeration = ((enumeration_t *)hash_get(enum_array, ((enumeration_t *)(bit_elem->data))->name));
                        linked_list_t *current_pattern = enumeration->pattern_list;
                        token_id = enumeration->token_id;
                        while (current_pattern != NULL)
                        {
                            darray_add(&rule_list_tint, token_id);
                            current_pattern = current_pattern->next;
                            token_id++;
                        }
                        // Close the set
                        token_id = -(int)']';
                        darray_add(&rule_list_tint, token_id);
                        break;
                    default:
                        fail_error("Undefined mnemonic type for the opcode '%s' (arg %i)", opcodes[i].text_pattern, j);
                        exit(EXIT_FAILURE);
                    }
                    break;
                }
                current = current->next;
            }
            found = (current != NULL);

            // Add an argument separator between each argument
            if (found)
                darray_add(&rule_list_tint, token_id_lookup[eT_ARG_SEPARATOR]);
        }

        // Remove all trailing argument separator
        // HACK: Find a better solution
        // 
        int last_token;
        do
        {
            darray_get(&rule_list_tint, &last_token, rule_list_tint->count - 1);
            if (last_token == token_id_lookup[eT_ARG_SEPARATOR])
                darray_remove(&rule_list_tint, 1);
        } while (last_token == token_id_lookup[eT_ARG_SEPARATOR]);

        // Should end with a new line or a comment
        // Open a set
        int control_token = -(int)'[';
        darray_add(&rule_list_tint, control_token);
        darray_add(&rule_list_tint, token_id_lookup[eT_COMMENT]);
        darray_add(&rule_list_tint, token_id_lookup[eT_NEWLINE]);
        // Close the set
        control_token = -(int)']';
        darray_add(&rule_list_tint, control_token);

        rule_def_t *new_rule = xmalloc(sizeof(rule_def_t) + rule_list_tint->element_size * rule_list_tint->count);
        new_rule->id = i;
        new_rule->count = rule_list_tint->count;
        new_rule->action = generator_generate_opcode_action(opcodes[i]);
        new_rule->name = ((token_def_t *)darray_get_ptr(&tokens, i))->name;
        memcpy(&(new_rule->tokens), darray_get_ptr(&rule_list_tint, 0), rule_list_tint->count * rule_list_tint->element_size);
        rules[i] = new_rule;
    }

    int x = 0;
    rule_def_t *new_rule = xmalloc(sizeof(rule_def_t) + 1);
    new_rule->id = opcode_count + x;
    new_rule->count = 1;
    new_rule->name = "empty_comment";
    new_rule->action = NULL;
    new_rule->tokens[0] = token_id_lookup[eT_COMMENT];
    rules[opcode_count + x] = new_rule;
    x++;

    new_rule = xmalloc(sizeof(rule_def_t) + 1);
    new_rule->id = opcode_count + x;
    new_rule->count = 1;
    new_rule->name = "empty_newline";
    new_rule->action = NULL;
    new_rule->tokens[0] = token_id_lookup[eT_NEWLINE];
    rules[opcode_count + x] = new_rule;
    x++;

    new_rule = xmalloc(sizeof(rule_def_t) + 1);
    new_rule->id = opcode_count + x;
    new_rule->count = 1;
    new_rule->name = "label";
    new_rule->action = action_label;
    new_rule->tokens[0] = token_id_lookup[eT_LABEL];
    rules[opcode_count + x] = new_rule;
    x++;

    new_rule = xmalloc(sizeof(rule_def_t) + 1);
    new_rule->id = opcode_count + x;
    new_rule->count = 1;
    new_rule->name = "address";
    new_rule->action = action_address;
    new_rule->tokens[0] = token_id_lookup[eT_ADDRESS];
    rules[opcode_count + x] = new_rule;
    x++;

    new_rule = xmalloc(sizeof(rule_def_t) + 1);
    new_rule->id = opcode_count + x;
    new_rule->count = 1;
    new_rule->name = "section";
    new_rule->action = NULL;
    new_rule->tokens[0] = token_id_lookup[eT_SECTION];
    rules[opcode_count + x] = new_rule;
    x++;

    generator_generate_parser(n_rules, rules);
}

char *name_from_pattern(const char *str)
{
    int len = strlen(str);
    char *name = xmalloc(len + 7 + 1);

    // Simply replace any non-alphanumeric char with an underscore
    for (size_t i = 0; i < len; i++)
    {
        name[i] = ((str[i] >= '0' && str[i] <= '9') || (str[i] >= 'a' && str[i] <= 'z') || (str[i] >= 'A' && str[i] <= 'Z'))
                      ? str[i]
                      : '_';
    }

    // Append a random value to the end
    sprintf(name + len, "_%06X", rand() & 0xFFFFFF);

    return name;
}