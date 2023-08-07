#include "generator.h"
#include "failure.h"

/*********************************************************************/

static FILE *fd = NULL;

static state_machine_t *lexer_dfa;
static const token_def_t *tokens_array;
static int token_count;

static state_machine_t *parser_dfa;
static const rule_def_t **rules;
static int rule_count;

/**
 * @brief print with an indentation level
 *
 * @param indentation Indentation level, one level is 4 spaces
 * @param format Format string
 * @param ... Argument for printf
 */
void iprintf(size_t indentation, const char *format, ...);

/**
 * @brief Generate an implementation of a state machine
 *
 * @param indent Indentation value
 * @param state_machine State machine to implement
 * @param name The name of the state machine, used in naming variables and functions
 */
void generator_dfa_switch(int indent, state_machine_t *state_machine, char *name);

// Handle an xmalloc error
static void xmalloc_callback(int err);

/*********************************************************************/

void generator_set_file_descriptor(FILE *file_descriptor)
{
    fd = file_descriptor;
}

void generator_generate_lexer(int count, const token_def_t *_tokens)
{
    token_count = count;
    tokens_array = _tokens;
    xmalloc_set_handler(xmalloc_callback);
    lexer_dfa = xmalloc(sizeof(state_machine_t));
    state_machine_t nfa = tokeniser_array_to_nfa(count, _tokens);
    state_machine_reduce(&nfa);
    *lexer_dfa = state_machine_make_deterministic(&nfa);
    state_machine_reduce(lexer_dfa);
    // state_machine_destroy(&nfa);
}

void generator_generate_parser(int count, const rule_def_t **_rules)
{
    fail_debug("Generating %i parser rules", count);

    rule_count = count;
    rules = _rules;
    xmalloc_set_handler(xmalloc_callback);
    parser_dfa = xmalloc(sizeof(state_machine_t));
    state_machine_t nfa = parser_arrays_to_nfa(count, _rules);
    state_machine_reduce(&nfa);
    *parser_dfa = state_machine_make_deterministic(&nfa);
    state_machine_reduce(parser_dfa);

    // state_machine_destroy(&nfa);
}

char *generator_generate_pattern_action(pattern_t *pattern)
{
    darray_t **buff = alloca(sizeof(sizeof(darray_t *)));
    *buff = darray_init(1);

    // Because of the way "darray_add" is implemented, we can't pass it rvalues
    char tmp = '\0';
    darray_add(buff, tmp); // Start the buffer as an empty string

    // TODO: free the dynamic array. Make "darray_destroy" first
    bprintf(buff, "    ASS_data_t data;\n\n");
    bprintf(buff, "    data.type = ASS_DT_SIGNED;\n\n");
    bprintf(buff, "    data.iVal = 0x%X;\n\n", pattern->bit_const.val);
    bprintf(buff, "    return data;\n\n");
    xmalloc_set_handler(xmalloc_callback);
    char *result = xmalloc((*buff)->count);
    strcpy(result, (char *)((*buff)->element_list));
}

char *generator_generate_opcode_action(opcode_t opcode)
{
    uint32_t offset = 0;
    darray_t **buff = alloca(sizeof(sizeof(darray_t *)));
    *buff = darray_init(1);

    fail_debug("Generating opcode action for opcode \"%s\"", opcode.text_pattern);

    // Because of the way "darray_add" is implemented, we can't pass it rvalues
    char tmp = '\0';
    darray_add(buff, tmp); // Start the buffer as an empty string

    bprintf(buff, "    ASS_opcode_t opcode =\n");
    bprintf(buff, "    {\n");
    bprintf(buff, "        .address = ASS_current_address,\n");
    bprintf(buff, "        .data = 0LLU\n");
    bprintf(buff, "    };\n");
    bprintf(buff, "\n");
    bprintf(buff, "    uint64_t data = 0;\n");
    bprintf(buff, "    uint64_t mask = 0;\n");
    bprintf(buff, "    ASS_ref_t new_ref;\n");
    bprintf(buff, "\n");

    // Process each element in the opcode reverse order
    //  doing it in reverse allow use to keep track of the
    //  offset
    int len = list_get_lenght(opcode.bit_pattern);
    for (int i = len - 1; i >= 0; i--)
    {
        linked_list_t *current = opcode.bit_pattern;
        while (current != NULL)
        {
            bit_elem_t *bit_elem = (bit_elem_t *)(current->user_data);

            if (bit_elem->index_opcode == i)
            {
                // Compute the bit mask
                uint64_t mask = (0xFFFFFFFFFFFFFFFFLLU << offset);
                mask &= ~(0xFFFFFFFFFFFFFFFFLLU << (offset + bit_elem->width));

                // Compute the value (for literals)
                uint64_t val = (((bit_const_t *)(bit_elem->data))->val << offset) & mask;

                switch (bit_elem->type)
                {
                case eBP_IMMEDIATE:
                    bprintf(buff, "    /**eBP_IMMEDIATE**/\n");
                    bprintf(buff, "    if (ASS_parser_stack[%i].type == ASS_DT_STRING)\n", bit_elem->index_mnemonic);
                    bprintf(buff, "        data = ASS_resolve_const(ASS_parser_stack[%i].sVal);\n", bit_elem->index_mnemonic);
                    bprintf(buff, "    else\n");
                    bprintf(buff, "        data = ASS_parser_stack[%i].iVal;\n", bit_elem->index_mnemonic);
                    // bprintf(buff, "    opcode.data &= 0x%llXLLU;\n", ~mask);
                    bprintf(buff, "    opcode.data |= (0x%llXLLU & (data << %u));\n", mask, offset);
                    break;
                case eBP_LABEL_ABS:
                    bprintf(buff, "    /**eBP_LABEL_ABS**/\n");
                    bprintf(buff, "    new_ref.absolute = true;\n");
                    bprintf(buff, "    new_ref.symbol_name = ASS_parser_stack[%i].sVal;\n", bit_elem->index_mnemonic);
                    bprintf(buff, "    new_ref.index = ASS_binary_stack_ptr;\n");
                    bprintf(buff, "    new_ref.bit_offset = %i;\n", offset);
                    bprintf(buff, "    new_ref.bit_width = %i;\n", bit_elem->width);
                    bprintf(buff, "    ASS_ref_stack_push(new_ref);\n");
                    break;
                case eBP_LABEL_REL:
                    bprintf(buff, "    /**eBP_LABEL_REL**/\n");
                    bprintf(buff, "    new_ref.absolute = false;\n");
                    bprintf(buff, "    new_ref.symbol_name = ASS_parser_stack[%i].sVal;\n", bit_elem->index_mnemonic);
                    bprintf(buff, "    new_ref.index =  ASS_binary_stack_ptr;\n");
                    bprintf(buff, "    new_ref.bit_offset = %i;\n", offset);
                    bprintf(buff, "    new_ref.bit_width = %i;\n", bit_elem->width);
                    bprintf(buff, "    ASS_ref_stack_push(new_ref);\n");
                case eBP_ENUM:
                    bprintf(buff, "    /**eBP_ENUM**/\n");
                    bprintf(buff, "    data = ASS_parser_stack[%i].iVal;\n", bit_elem->index_mnemonic);
                    // bprintf(buff, "    opcode.data &= 0x%llXLLU;\n", ~mask);
                    bprintf(buff, "    opcode.data |= (0x%llXLLU & (data << %u));\n", mask, offset);
                    break;
                case eBP_ID:
                    fprintf(stderr, "Unresolved ID\n");
                    abort();
                    break;
                case eBP_BIT_CONST:
                case eBP_BIT_LIT:
                    bprintf(buff, "    /**eBP_BIT_LIT**/\n");
                    // bprintf(buff, "    opcode.data &= 0x%llXLLU;\n", ~mask);
                    bprintf(buff, "    opcode.data |= (0x%llXLLU);\n", val);
                    break;
                case eBP_ELLIPSIS:
                    bprintf(buff, "    /**eBP_ELLIPSIS**/\n");
                    // bprintf(buff, "    opcode.data &= 0x%llXLLU;\n", ~mask);
                    break;
                default:
                    fail_error("Unknown bit pattern type");
                    exit(EXIT_FAILURE);
                    break;
                }

                fail_debug("Element %i (%s) offset %u width %u", i, name_BPTYPE[bit_elem->type], offset, bit_elem->width);

                // Keep track of the offset
                offset += bit_elem->width;
                break;
            }

            current = current->next;
        }
    }

    // TODO: free the dynamic array. Make "darray_destroy" first
    bprintf(buff, "\n");
    bprintf(buff, "    ASS_binary_stack_push(opcode);\n");
    bprintf(buff, "    ASS_current_address++;\n");
    xmalloc_set_handler(xmalloc_callback);
    char *result = xmalloc((*buff)->count);
    strcpy(result, (char *)((*buff)->element_list));

    return result;
}

/**************************************************/
/*                   CALLBACKS                    */
/**************************************************/

void generator_default_macros(int indent)
{
    int count = hash_count(macro_array);
    bucket_t **macro = hash_serialise(macro_array);

    iprintf(0, "// Default macros");

    for (int i = 0; i < count; i++)
    {
        // Get the macro from the bucket
        macro_t *macro_elem = (macro_t *)(macro[i]->user_data);

        iprintf(0 + indent, "ASS_insert_macro((ASS_macro_t){.name = \"%s\", .content = \"%s\"});", macro_elem->name, macro_elem->content);
    }
}

void generator_custom_outputs_selection(int indent)
{
    iprintf(0, "");
    custom_output_t *array = darray_get_ptr(&custom_output_array, 0);
    for (int i = 0; i < custom_output_array->count; i++)
    {
        iprintf(0 + indent, "else if (strcmp(argv[i], \"%s\") == 0)", array[i].name);
        iprintf(0 + indent, "{");
        iprintf(0 + indent, "    ASS_output_format = ASS_OUT_%s;", array[i].name);
        iprintf(0 + indent, "}");
    }
}

void generator_custom_outputs_switch(int indent)
{
    iprintf(0, "");
    custom_output_t *array = darray_get_ptr(&custom_output_array, 0);
    for (int i = 0; i < custom_output_array->count; i++)
    {
        iprintf(1 + indent, "case ASS_OUT_%s:", array[i].name);
        iprintf(1 + indent, "ASS_output_%s(fd);", array[i].name);
        iprintf(1 + indent, "break;");
    }
}

void generator_outputs_enum(int indent)
{
    iprintf(0, "enum");
    iprintf(0 + indent, "{");
    iprintf(1 + indent, "ASS_OUT_UNKNOWN,");
    iprintf(1 + indent, "ASS_OUT_HEX,");
    iprintf(1 + indent, "ASS_OUT_COE,");
    iprintf(1 + indent, "ASS_OUT_BIT,");
    iprintf(1 + indent, "ASS_OUT_VHDL,");

    custom_output_t *array = darray_get_ptr(&custom_output_array, 0);
    for (int i = 0; i < custom_output_array->count; i++)
    {
        iprintf(1 + indent, "ASS_OUT_%s,", array[i].name);
    }

    iprintf(0 + indent, "};");
}

void generator_custom_outputs_function(int indent)
{
    iprintf(0, "");

    custom_output_t *array = darray_get_ptr(&custom_output_array, 0);
    for (int i = 0; i < custom_output_array->count; i++)
    {
        iprintf(0 + indent, "void ASS_output_%s(FILE* fd)", array[i].name);
        iprintf(0 + indent, "{");
        iprintf(1 + indent, "%s", array[i].code);
        iprintf(0 + indent, "}");
    }
}

void generator_startup(int indent)
{
    if (override_code[0] != NULL)
    {
        iprintf(0, "void ASS_startup()");
        iprintf(0 + indent, "{");
        iprintf(1 + indent, "%s", override_code[0]);
        iprintf(0 + indent, "}");
    }
    else
    {
        iprintf(0, "void ASS_startup(){} // Empty");
    }
}

void generator_custom_code(int indent)
{
    iprintf(0, code);
}

void generator_help_message(int indent)
{
    iprintf(0, "char const *const ASS_help_message =");
    iprintf(1 + indent, "\"Usage: %%s [OPTION]... -o OUTPUT_FILE INPUT_FILE\\n\"");
    iprintf(1 + indent, "\"%s\\n\"", parameters.description);
    iprintf(1 + indent, "\"\\n\"");
    iprintf(1 + indent, "\"Options:\\n\"");
    iprintf(1 + indent, "\"  -o <FILE>    set the output file\\n\"");
    iprintf(1 + indent, "\"  -h           display this help and exit\\n\"");
    iprintf(1 + indent, "\"  -V           output version information and exit\\n\"");
    iprintf(1 + indent, "\"  -v           verbose\\n\"");
    iprintf(1 + indent, "\"  -c           disable coloured messages\\n\"");
    iprintf(1 + indent, "\"  -f <FORMAT>  set the format for the output file\\n\"");
    iprintf(1 + indent, "\"\\n\"");
    iprintf(1 + indent, "\"FORMAT is the format of the output file. The available formats are:\\n\"");
    iprintf(1 + indent, "\"  hex          Intel HEX (default)\\n\"");
    iprintf(1 + indent, "\"  coe          Xilinx COE\\n\"");
    iprintf(1 + indent, "\"  vhdl         VHDL data array\\n\"");
    iprintf(1 + indent, "\"  bit          Raw data\\n\"");

    // Display help for custom output formats
    custom_output_t *array = darray_get_ptr(&custom_output_array, 0);
    for (int i = 0; i < custom_output_array->count; i++)
    {
        iprintf(1 + indent, "\"  %-12s %s\\n\"", array[i].name, array[i].description);
    }
    iprintf(1 + indent, ";");
}

void generator_version_message(int indent)
{
    iprintf(1 + indent, "char const *const ASS_version_message =");
    iprintf(1 + indent, "\"%s %s\\n\"", parameters.name, parameters.version);
    iprintf(1 + indent, "\"Written by %s.\\n\"", parameters.author);
    iprintf(1 + indent, "\"\\n\"");
    iprintf(1 + indent, "\"Copyright (C) %s.\\n\"", parameters.copyright);
    iprintf(1 + indent, "\"This program comes with ABSOLUTELY NO WARRANTY; not even for\\n\"");
    iprintf(1 + indent, "\"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\\n\"");
    iprintf(1 + indent, "\"This is free software, and you are welcome to redistribute it\\n\"");
    iprintf(1 + indent, "\"under certain conditions; refer to the license for details.\\n\";");
}

void generator_notice(int indent)
{
    iprintf(0, "/* A free assembler, made by ASS %i.%i.%i */", VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION);
}

void generator_parameters(int indent)
{
    iprintf(0, "int64_t const ASS_P_opcode_width = %lli;", parameters.opcode_width);
    iprintf(0 + indent, "int64_t const ASS_P_memory_width = %lli;", parameters.memory_width);
    iprintf(0 + indent, "int64_t const ASS_P_alignment = %lli;", parameters.alignment);
    iprintf(0 + indent, "int64_t const ASS_P_address_width = %lli;", parameters.address_width);
    iprintf(0 + indent, "int64_t const ASS_P_address_start = %lli;", parameters.address_start);
    iprintf(0 + indent, "int64_t const ASS_P_address_stop = %lli;", parameters.address_stop);
    iprintf(0 + indent, "int const ASS_P_endianness = %i;", parameters.endianness);
    iprintf(0 + indent, "char const ASS_P_args_separator = '%c';", parameters.args_separator);
    iprintf(0 + indent, "char const ASS_P_label_postfix = '%c';", parameters.label_postfix);
}

void generator_data_union(int indent)
{
    iprintf(0, "typedef struct");
    iprintf(0 + indent, "{");
    iprintf(1 + indent, "ASS_DT_t type;");
    iprintf(1 + indent, "union {");
    iprintf(2 + indent, "uint64_t uVal;");
    iprintf(2 + indent, "int64_t iVal;");
    iprintf(2 + indent, "char *sVal;");
    iprintf(1 + indent, "};");
    iprintf(0 + indent, "} ASS_data_t;");
}

void generator_data_types(int indent)
{
    iprintf(0, "typedef enum {");
    iprintf(1 + indent, "ASS_DT_NULL,");
    iprintf(1 + indent, "ASS_DT_UNSIGNED,");
    iprintf(1 + indent, "ASS_DT_SIGNED,");
    iprintf(1 + indent, "ASS_DT_STRING,");
    iprintf(1 + indent, "} ASS_DT_t;");
}

void generator_lexer_actions(int indent)
{
    int duplicates[token_count];

    for (size_t i = 0; i < token_count; i++)
    {
        // Look if we have already genrated the token id
        duplicates[i] = -1;
        bool duplicated = false;
        for (size_t j = 0; j < i; j++)
        {
            if (duplicates[j] == tokens_array[i].id)
            {
                duplicated = true;
                break;
            }
        }

        // Save the the id
        duplicates[i] = tokens_array[i].id;

        // Generate the token id if it is not a duplicate
        if (!duplicated)
        {
            if (tokens_array[i].action != NULL)
            {
                iprintf(0 + indent, "ASS_data_t ASS_TA_%s()", tokens_array[i].name);
                iprintf(0 + indent, "{");
                iprintf(0, "%s", tokens_array[i].action);
                iprintf(0 + indent, "}");
            }
            else
            {
                iprintf(0 + indent, "// Empty action");
                iprintf(0 + indent, "ASS_data_t ASS_TA_%s()", tokens_array[i].name);
                iprintf(0 + indent, "{");
                iprintf(1 + indent, "return (ASS_data_t){ASS_DT_NULL, (uint64_t)0};");
                iprintf(0 + indent, "}");
            }
        }
    }
}

void generator_lexer_action_list(int indent)
{
    int duplicates[token_count];

    iprintf(0, "const ASS_action_t ASS_lexer_action_list[] = ");
    iprintf(0 + indent, "{");
    for (size_t i = 0; i < token_count; i++)
    {
        // Look if we have already genrated the token id
        duplicates[i] = -1;
        bool duplicated = false;
        for (size_t j = 0; j < i; j++)
        {
            if (duplicates[j] == tokens_array[i].id)
            {
                duplicated = true;
                break;
            }
        }

        // Save the the id
        duplicates[i] = tokens_array[i].id;

        // Generate the token id if it is not a duplicate
        if (!duplicated)
        {
            iprintf(1 + indent,
                    "[ASS_T_%s] = (ASS_action_t){.action = ASS_TA_%s, .type = %s},",
                    tokens_array[i].name,
                    tokens_array[i].name,
                    tokens_array[i].action == NULL ? "ASS_U_NONE" : "ASS_U_DATA");
        }
    }
    iprintf(0 + indent, "};");
}

void generator_parser_actions(int indent)
{
    int duplicates[rule_count];

    for (size_t i = 0; i < rule_count; i++)
    {
        // Look if we have already generated the rule id
        duplicates[i] = -1;
        bool duplicated = false;
        for (size_t j = 0; j < i; j++)
        {
            if (duplicates[j] == rules[i]->id)
            {
                duplicated = true;
                break;
            }
        }

        // Save the the id
        duplicates[i] = rules[i]->id;

        // Generate the rule id if it is not a duplicate
        if (!duplicated)
        {
            if (rules[i]->action != NULL)
            {
                iprintf(0 + indent, "// %s action", rules[i]->name);
                iprintf(0 + indent, "ASS_data_t ASS_RA_%s()", rules[i]->name);
                iprintf(0 + indent, "{");
                iprintf(0, "%s", rules[i]->action);
                iprintf(0 + indent, "}");
            }
            else
            {
                iprintf(0 + indent, "// Empty action");
                iprintf(0 + indent, "ASS_data_t ASS_RA_%s()", rules[i]->name);
                iprintf(0 + indent, "{");
                iprintf(1 + indent, "return (ASS_data_t){ASS_DT_NULL, (uint64_t)0};");
                iprintf(0 + indent, "}");
            }
        }
    }
}

void generator_parser_action_list(int indent)
{
    int duplicates[rule_count];

    iprintf(0, "const ASS_action_t ASS_parser_action_list[] = ");
    iprintf(0 + indent, "{");
    for (size_t i = 0; i < rule_count; i++)
    {
        // Look if we have already genrated the rule id
        duplicates[i] = -1;
        bool duplicated = false;
        for (size_t j = 0; j < i; j++)
        {
            if (duplicates[j] == rules[i]->id)
            {
                duplicated = true;
                break;
            }
        }

        // Save the the id
        duplicates[i] = rules[i]->id;

        // Generate the rule id if it is not a duplicate
        if (!duplicated)
            iprintf(1 + indent, "[%i] = (ASS_action_t){.action = ASS_RA_%s, .type = ASS_U_NONE},", rules[i]->id, rules[i]->name);
    }
    iprintf(0 + indent, "};");
}

void generator_token_enum(int indent)
{
    int duplicates[token_count];

    iprintf(0, "typedef enum");
    iprintf(0 + indent, "{");
    for (size_t i = 0; i < token_count; i++)
    {
        // Look if we have already genrated the token id
        duplicates[i] = -1;
        bool duplicated = false;
        for (size_t j = 0; j < i; j++)
        {
            if (duplicates[j] == tokens_array[i].id)
            {
                duplicated = true;
                break;
            }
        }

        // Save the the id
        duplicates[i] = tokens_array[i].id;

        // Generate the token id if it is not a duplicate
        if (!duplicated)
            iprintf(1 + indent, "ASS_T_%s = %i,", tokens_array[i].name, tokens_array[i].id);
    }
    iprintf(0 + indent, "} ASS_token_t;");
}

void generator_token_names(int indent)
{
    iprintf(0, "const char const *ASS_token_names[] = {");
    for (size_t i = 0; i < token_count; i++)
    {
        iprintf(1 + indent, "[ASS_T_%s] = \"%s\",", tokens_array[i].name, tokens_array[i].name);
    }
    iprintf(0 + indent, "};");
}

void generator_lexer_switch(int indent)
{
    generator_dfa_switch(indent, lexer_dfa, "lexer");
}

void generator_parser_switch(int indent)
{
    generator_dfa_switch(indent, parser_dfa, "parser");
}

void generator_dfa_switch(int indent, state_machine_t *state_machine, char *name)
{
    iprintf(0, "switch (ASS_%s_state)", name);
    iprintf(1 + indent, "{");

    for (size_t i = 0; i < state_machine->states_tstate->count; i++)
    {
        transistion_t default_transition = {.condition = -1, .next_state_id = -1};
        state_t *state = darray_get_ptr(&(state_machine->states_tstate), i);

        iprintf(0 + indent, "case %i:", state->id);

        iprintf(1 + indent, "switch (ASS_%s_token)", name);
        iprintf(1 + indent, "{");

        for (size_t j = 0; j < state->transitions_ttrans->count; j++)
        {
            transistion_t *transition = darray_get_ptr(&(state->transitions_ttrans), j);

            if (transition->condition == CONDITION_ANY) // default_transition (any token)
            {
                if (default_transition.next_state_id >= 0)
                    fail_error("A state has multiple default transitions");
                default_transition = *transition;
                continue;
            }

            iprintf(1 + indent, "case %i:", transition->condition);
            if (j >= state->transitions_ttrans->count || transition->next_state_id != (transition + 1)->next_state_id)
            {
                iprintf(2 + indent, "ASS_%s_state = %i;", name, transition->next_state_id);
                iprintf(2 + indent, "ASS_%s_valid = %s;", name, state_machine_get_by_id(state_machine, transition->next_state_id)->end_state ? "true" : "false");
                iprintf(2 + indent, "ASS_%s_output = %i;", name, state_machine_get_by_id(state_machine, transition->next_state_id)->output);
                iprintf(2 + indent, "break;");
            }
        }

        iprintf(1 + indent, "default:");
        if (default_transition.next_state_id >= 0)
        {
            // Default transition
            fail_debug("next_state_id : %i", default_transition.next_state_id);
            iprintf(2 + indent, "//Default transition");
            iprintf(2 + indent, "ASS_%s_state = %i;", name, default_transition.next_state_id);
            iprintf(2 + indent, "ASS_%s_valid = %s;", name, state_machine_get_by_id(state_machine, default_transition.next_state_id)->end_state ? "true" : "false");
            iprintf(2 + indent, "ASS_%s_output = %i;", name, state_machine_get_by_id(state_machine, default_transition.next_state_id)->output);
        }
        else
        {
            // No default transition
            iprintf(2 + indent, "if(ASS_%s_valid)", name);
            iprintf(3 + indent, "ASS_%s_exit_point();", name);
            iprintf(2 + indent, "else");
            iprintf(3 + indent, "ASS_%s_invalid_token();", name);
        }
        iprintf(2 + indent, "break;");
        iprintf(1 + indent, "}");
        iprintf(1 + indent, "break;");
    }

    iprintf(0 + indent, "default:");
    iprintf(1 + indent, "fprintf(stderr, \"state machine error\\n\");");
    iprintf(1 + indent, "abort();");
    iprintf(0 + indent, "}");
}

/*********************************************************************/

void iprintf(size_t indentation, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    for (size_t i = 0; i < indentation; i++)
    {
        fputs("    ", fd);
    }
    vfprintf(fd, format, args);
    fputc('\n', fd);
    va_end(args);
}

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