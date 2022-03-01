#include "tokeniser.h"

#include "failure.h"

state_machine_t tokeniser_array_to_nfa(int count, const token_def_t *tokens_array)
{
    state_machine_t new_state_machine;
    state_machine_t merged_state_machine;

    // Generate the first fsm
    merged_state_machine = tokeniser_token_to_nfa(tokens_array[0]);

    for (size_t i = 1; i < count; i++)
    {
        // Generate a new state machine and reduce if possible
        new_state_machine = tokeniser_token_to_nfa(tokens_array[i]);
        merged_state_machine = state_machine_merge(&merged_state_machine, &new_state_machine);
        state_machine_reduce(&merged_state_machine);
    }

    return merged_state_machine;
}

// Generate a state machine matching the provided string
state_machine_t tokeniser_token_to_nfa(const token_def_t token)
{
    darray_t* sequence = darray_init(sizeof(int));
    int processed_char;

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

    // Process the pattern
    bool escaped = false;
    for (size_t i = 0; i < len; i++)
    {
        if (escaped)
        {
            switch (string[i])
            {
            case '`':  //GNU's POSIX extension for EOF
            case '\'':
                processed_char = -1; //EOF
                break;
            case 'a':
                processed_char = '\a';
                break;
            case 'b':
                processed_char = '\b';
                break;
            case 't':
                processed_char = '\t';
                break;
            case 'n':
                processed_char = '\n';
                break;
            case 'v':
                processed_char = '\v';
                break;
            case 'f':
                processed_char = '\f';
                break;
            case 'r':
                processed_char = '\r';
                break;
            case 'x': // Enter hex escape sequence
                char *new_ptr;
                processed_char = strtoul(string + i + 1, &new_ptr, 16);
                i = (int)((ptrdiff_t)new_ptr - (ptrdiff_t)string) - 1;
                break;
            case '0':// Enter octal escape sequence
            case '1':
            case '2':
            case '3':
                processed_char = (string[i++] << 6) & 0300;
                processed_char |= (string[i++] << 3) & 0070;
                processed_char |= (string[i]) & 0007;
                break;
            case '[':
                processed_char = '[';
                break;
            case ']':
                processed_char = ']';
                break;
            case '(':
                processed_char = '(';
                break;
            case ')':
                processed_char = ')';
                break;
            case '.':
                processed_char = '.';
                break;
            case '+':
                processed_char = '+';
                break;
            case '*':
                processed_char = '*';
                break;
            case '?':
                processed_char = '?';
            case '-':
                processed_char = '-';
                break;
            default:
                fail_error("Invalid escape sequence. (\\%c)", string[i]);
                break;
            }
            escaped = false;
        }
        else
        {
            switch (string[i])
            {
            case '\\': //Enter an escape sequence
                escaped = true;
                continue; //Don't add the escape character to the sequence
            case '+':
                processed_char = -(int)'+';
                break;
            case '*':
                processed_char = -(int)'*';
                break;
            case '?':
                processed_char = -(int)'*';
                break;
            case '-':
                processed_char = -(int)'-';
                break;
            case '[':
                processed_char = -(int)'[';
                break;
            case ']':
                processed_char = -(int)']';
                break;
            case '(':
                processed_char = -(int)'(';
                break;
            case ')':
                processed_char = -(int)')';
                break;
            case '|':
                processed_char = -(int)'|';
                break;
            default:
                processed_char = string[i];
                break;
            }
        }
        darray_add(&sequence, processed_char);
    }
    return pattern_compiler(sequence->count, (int*)darray_get_ptr(&sequence, 0), token.id);
}