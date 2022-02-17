#include "generator.h"

/*********************************************************************/

static FILE *fd = NULL;
static state_machine_t *state_machine;
static state_machine_t *lexer_dfa;
static const token_def_t *tokens;
static int tokens_count;

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
    tokens_count = count;
    tokens = _tokens;
    xmalloc_set_handler(xmalloc_callback);
    lexer_dfa = xmalloc(sizeof(state_machine_t));
    state_machine_t nfa = tokeniser_array_to_nfa(count, _tokens);
    state_machine_reduce(&nfa);
    *lexer_dfa = state_machine_make_deterministic(&nfa);
    state_machine_reduce(lexer_dfa);
    // state_machine_destroy(&nfa);
}

/**************************************************/
/*                   CALLBACKS                    */
/**************************************************/

void generator_token_enum(int indent)
{
    iprintf(0, "typedef enum");
    iprintf(0 + indent, "{");
    for (size_t i = 0; i < tokens_count; i++)
    {
        iprintf(1 + indent, "ASS_T_%s = %i,", tokens[i].name, tokens[i].id);
    }
    iprintf(0 + indent, "} ASS_token_t;");
}

void generator_token_names(int indent)
{
    iprintf(0, "{");
    for (size_t i = 0; i < tokens_count; i++)
    {
        iprintf(1 + indent, "[ASS_T_%s] = \"%s\",", tokens[i].name, tokens[i].name);
    }
    iprintf(0 + indent, "};");
}

void generator_lexer_switch(int indent)
{
    generator_dfa_switch(indent, lexer_dfa, "lexer");
}

void generator_dfa_switch(int indent, state_machine_t *state_machine, char *name)
{
    iprintf(0, "switch (ASS_%s_state)", name);
    iprintf(1 + indent, "{");

    for (size_t i = 0; i < state_machine->states_tstate->count; i++)
    {
        state_t *state = darray_get_ptr(&(state_machine->states_tstate), i);

        iprintf(0 + indent, "case %i:", state->id);
        iprintf(1 + indent, "switch (ASS_%s_token)", name);
        iprintf(1 + indent, "{");

        for (size_t j = 0; j < state->transitions_ttrans->count; j++)
        {
            transistion_t *transition = darray_get_ptr(&(state->transitions_ttrans), j);
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
        iprintf(2 + indent, "if(ASS_%s_valid)", name);
        iprintf(3 + indent, "ASS_%s_exit_point();", name);
        iprintf(2 + indent, "else");
        iprintf(3 + indent, "ASS_%s_invalid_token();", name);
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