#include "lexer.h"

#include "failure.h"

// Default token actions
char *action_parse_uint =
    "    ASS_data_t data;\n"
    "    data.uVal = strtoull(ASS_text, NULL, 0);\n" // strtoull requires C11
    "    data.type = ASS_DT_UNSIGNED;\n"
    "    return data;";

char *action_parse_int =
    "    ASS_data_t data;\n"
    "    data.iVal = strtoll(ASS_text, NULL, 0);\n" // strtoll requires C11
    "    data.type = ASS_DT_SIGNED;\n"
    "    return data;";

char *action_parse_char =
    "    ASS_data_t data;\n"
    "    data.iVal = (uint64_t)ASS_text[1];\n"
    "    data.type = ASS_DT_SIGNED;\n"
    "    return data;";

char *action_parse_str =
    "    ASS_data_t data;\n"
    "    data.sVal = malloc(strlen(ASS_text) + 1);\n"
    "    data.type = ASS_DT_STRING;\n"
    "    strcpy(data.sVal, ASS_text);\n"
    "    return data;";

// Token id lookup
int token_id_lookup[] =
    {
        [eT_ARG_SEPARATOR] = -1,
        [eT_COMMENT] = -1,
        [eT_NEWLINE] = -1,
        [eT_WHITESPACE] = -1,
        [eT_ADDRESS] = -1,
        [eT_LABEL] = -1,
        [eT_IMMEDIATE_INT] = -1,
        [eT_IMMEDIATE_CHAR] = -1,
        [eT_IDENTIFIER] = -1,
        [eT_CONSTANT_DIR] = -1,
};

darray_t *tokens;

static char *name_from_pattern(const char *str);

void lexer_init()
{
    tokens = darray_init(sizeof(token_def_t));
}

void lexer_generate()
{
    int id = 0;
    token_def_t new_token;

    //Serialise the opcodes
    int count = opcode_array->count;
    opcode_t *opcodes = darray_get_ptr(&opcode_array, 0);

    // Comments have priority over any token
    token_id_lookup[eT_COMMENT] = id;
    new_token = (token_def_t){.name = "COMMENT", .id = id++, .pattern = ";[\\t -~]*", .action = NULL};
    darray_add(&tokens, new_token);

    // Mnemonic tokens
    int opcode_count = opcode_array->count;
    opcodes = darray_get_ptr(&opcode_array, 0);

    for (size_t i = 0; i < opcode_count; i++)
    {
        new_token.id = id++;
        new_token.pattern = opcodes[i].text_pattern;
        new_token.name = name_from_pattern(opcodes[i].text_pattern);
        new_token.action = NULL;
        new_token.data = &(opcodes[i]); // Store a reference to the original opcode
        darray_add(&tokens, new_token);

        opcodes[i].token_id = new_token.id;
    }

    // Enum tokens
    count = hash_count(enum_array);
    bucket_t **enums = hash_serialise(enum_array);
    for (size_t i = 0; i < count; i++)
    {
        // Create on token per pattern, with one id per enum
        ((enumeration_t *)(enums[i]->user_data))->token_id = id; // Save the first token id
        linked_list_t *current = ((enumeration_t *)(enums[i]->user_data))->pattern_list;
        while (current != NULL)
        {
            new_token = (token_def_t){
                .name = name_from_pattern(((pattern_t *)(current->user_data))->pattern),
                .id = id,
                .pattern = ((pattern_t *)(current->user_data))->pattern,
                .action = generator_generate_pattern_action((pattern_t *)(current->user_data)),
                .data = (current->user_data)};
            darray_add(&tokens, new_token);
            current = current->next;
            id++;
        }
    }

    // Standard tokens
    token_id_lookup[eT_ARG_SEPARATOR] = id;
    new_token = (token_def_t){.name = "ARG_SEPARATOR", .id = id++, .pattern = xmalloc(2), .action = NULL};
    new_token.pattern[0] = parameters.args_separator;
    new_token.pattern[1] = '\0';
    darray_add(&tokens, new_token);

    token_id_lookup[eT_NEWLINE] = id;
    new_token = (token_def_t){.name = "NEWLINE", .id = id++, .pattern = "[\\n\\r\\']", .action = NULL}; // \' is EOF (-1)
    darray_add(&tokens, new_token);

    token_id_lookup[eT_WHITESPACE] = id;
    new_token = (token_def_t){.name = "WHITESPACE", .id = id++, .pattern = "[ \\t]+", .action = NULL};
    darray_add(&tokens, new_token);

    // TODO: make pattern a parameter
    token_id_lookup[eT_ADDRESS] = id;
    new_token = (token_def_t){.name = "ADDRESS", .id = id++, .pattern = xmalloc(strlen("0x[0-9a-fA-F]+:")), .action = action_parse_uint};
    strcpy(new_token.pattern, "0x[0-9a-fA-F]+:");
    new_token.pattern[strlen(new_token.pattern) - 1] = parameters.label_postfix;
    darray_add(&tokens, new_token);

    // TODO: make pattern a parameter
    token_id_lookup[eT_LABEL] = id;
    new_token = (token_def_t){.name = "LABEL", .id = id++, .pattern = xmalloc(strlen("[a-zA-Z_][0-9a-zA-Z_]*:")), .action = action_parse_str};
    strcpy(new_token.pattern, "[a-zA-Z_][0-9a-zA-Z_]*:");
    new_token.pattern[strlen(new_token.pattern) - 1] = parameters.label_postfix;
    darray_add(&tokens, new_token);

    // TODO: add signed numbers support
    // FIXME : only matches hex numbers, should match anything strtoull can parse
    token_id_lookup[eT_IMMEDIATE_INT] = id;
    new_token = (token_def_t){.name = "IMMEDIATE_INT", .id = id++, .pattern = "0x[0-9a-fA-F]+", .action = action_parse_int};
    darray_add(&tokens, new_token);

    token_id_lookup[eT_IMMEDIATE_CHAR] = id;
    new_token = (token_def_t){.name = "IMMEDIATE_CHAR", .id = id++, .pattern = "'[ -~]'", .action = action_parse_char};
    darray_add(&tokens, new_token);

    token_id_lookup[eT_CONSTANT_DIR] = id;
    new_token = (token_def_t){.name = "CONSTANT_DIR", .id = id++, .pattern = parameters.constant_dir, .action = NULL};
    darray_add(&tokens, new_token);

    token_id_lookup[eT_IDENTIFIER] = id;
    new_token = (token_def_t){.name = "IDENTIFIER", .id = id++, .pattern = "[a-zA-Z_][a-zA-Z0-9_]*", .action = action_parse_str};
    darray_add(&tokens, new_token);

    fail_detail("****Tokens****");
    for (size_t i = 0; i < tokens->count; i++)
    {
        new_token = *((token_def_t *)darray_get_ptr(&tokens, i));
        fail_detail("  Name : %s | Id : %i | Pattern : %s", new_token.name, new_token.id, new_token.pattern);
    }
    fail_detail("**************");

    generator_generate_lexer(tokens->count, (token_def_t *)tokens->element_list);
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