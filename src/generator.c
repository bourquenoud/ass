#include "generator.h"

/*********************************************************************/

static FILE *fd = NULL;
static state_machine_t *state_machine;

/**
 * @brief print with an indentation level
 *
 * @param indentation Indentation level, one level is 4 spaces
 * @param format Format string
 * @param ... Argument for printf
 */
void iprintf(size_t indentation, const char *format, ...);

/*********************************************************************/

void generator_set_file_descriptor(FILE *file_descriptor)
{
    fd = file_descriptor;
}

void generator_set_state_machine(state_machine_t *fsm)
{
    state_machine = fsm;
}

void generator_dfa_switch(int indent)
{
    iprintf(0, "switch (ASS_current_state)");
    iprintf(1 + indent, "{");

    for (size_t i = 0; i < state_machine->states_tstate->count; i++)
    {
        state_t *state = darray_get_ptr(&(state_machine->states_tstate), i);

        iprintf(1 + indent, "case %i:", state->id);
        iprintf(2 + indent, "switch (ASS_current_token)");
        iprintf(3 + indent, "{");

        for (size_t j = 0; j < state->transitions_ttrans->count; j++)
        {
            transistion_t *transition = darray_get_ptr(&(state->transitions_ttrans), j);
            iprintf(3 + indent, "case %i:", transition->condition);
            iprintf(4 + indent, "ASS_current_state = %i;", transition->next_state_id);
            iprintf(2 + indent, "ASS_valid_sentence = %s;", state_machine_get_by_id(state_machine, transition->next_state_id)->end_state ? "true" : "false");
            iprintf(4 + indent, "break;");
        }

        iprintf(3 + indent, "default:");
        iprintf(4 + indent, "ASS_syntax_error();");
        iprintf(4 + indent, "break;");
        iprintf(3 + indent, "}");
        iprintf(2 + indent, "break;");
    }

    iprintf(1 + indent, "default:");
    iprintf(2 + indent, "fprintf(stderr, \"state machine error\\n\");");
    iprintf(2 + indent, "abort();");
    iprintf(1 + indent, "}");
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