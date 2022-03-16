#include "populator.h"

#include "failure.h"
#include "generated/skeleton.h"

#define MAX_NAME_LENGHT 64

// Automatically add "generator_" before the name
#define register_function(name) hash_add(function_table, #name, generator_##name)

typedef void (*callable_t)(int);
hash_t *function_table;

void generate(FILE *fd)
{
    int wait_index = 3;

    function_table = hash_init(64);

    register_function(startup);
    register_function(custom_code);
    register_function(help_message);
    register_function(version_message);
    register_function(notice);
    register_function(parameters);
    register_function(data_union);
    register_function(data_types);
    register_function(lexer_actions);
    register_function(lexer_action_list);
    register_function(lexer_switch);
    register_function(parser_actions);
    register_function(parser_action_list);
    register_function(parser_switch);
    register_function(token_enum);
    register_function(token_names);

    int line = 1;
    int column = 1;

    char name_buff[MAX_NAME_LENGHT];
    // Sliding buffer
    char sl_buff[4];
    for (size_t i = 0; i < SKELETON_LEN; i++)
    {
        sl_buff[0] = sl_buff[1];
        sl_buff[1] = sl_buff[2];
        sl_buff[2] = sl_buff[3];
        sl_buff[3] = SKELETON[i];

        if (SKELETON[i] == '\n')
        {
            column = 1;
            line++;
        }
        else
        {
            column++;
        }

        // If we match a replace pattern, read the target name
        if (sl_buff[0] == '/' && sl_buff[1] == '*' && sl_buff[2] == '!' && sl_buff[3] == '!')
        {
            // Skip leading spaces
            int index = 0;
            char c = SKELETON[++i];
            while (isspace(c))
            {
                if (i == SKELETON_LEN - 1 || c == '\n')
                {
                    fail_error("Line %i, col %i : Syntax error in the skelton file, no name in replace pattern", line, column);
                    fail_error("Please report the issue to https://github.com/bourquenoud/ass");
                    abort();
                }
                c = SKELETON[++i];
            }

            // Read the name until we match an invalid character
            while (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c >= '0' && c <= '9' || c == '_')
            {
                name_buff[index++] = c;
                if (i == SKELETON_LEN - 1 || index >= MAX_NAME_LENGHT - 1 || c == '\n')
                {
                    fail_error("Line %i, col %i : Syntax error in the skelton file, replace pattern never closed or name too long", line, column);
                    fail_error("Please report the issue to https://github.com/bourquenoud/ass");
                    abort();
                }
                c = SKELETON[++i];
            }
            name_buff[index] = '\0';

            // Read until we match the closing pattern
            while (sl_buff[0] != '!' || sl_buff[1] != '!' || sl_buff[2] != '*' || sl_buff[3] != '/')
            {
                sl_buff[0] = sl_buff[1];
                sl_buff[1] = sl_buff[2];
                sl_buff[2] = sl_buff[3];
                sl_buff[3] = c;
                if (i == SKELETON_LEN - 1 || c == '\n')
                {
                    fail_error("Line %i, col %i : Syntax error in the skelton file, replace pattern never closed", line, column);
                    fail_error("Please report the issue to https://github.com/bourquenoud/ass");
                    abort();
                }
                c = SKELETON[++i];
            }

            // Execute the function
            callable_t function = hash_get(function_table, name_buff);
            if (function == NULL)
            {
                fail_error("Line %i, col %i : Syntax error in the skelton file, '%s' not registered", line, column, name_buff);
                fail_error("Please report the issue to https://github.com/bourquenoud/ass");
                abort();
            }
            function(column / 4 - 1);

            // Wait for the buffer to refill before printing again
            wait_index = i + 4;
        }
        else if (i >= wait_index)
        {
            putc(sl_buff[0], fd);
        }
    }

    // Flush the buffer
    putc(sl_buff[1], fd);
    putc(sl_buff[2], fd);
    putc(sl_buff[3], fd);
}