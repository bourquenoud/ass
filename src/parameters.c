#include "parameters.h"

#include <string.h>
#include <stdio.h>

#include "ast_node.h"
#include "generated/ass.tab.h"

#include "linked_list.h"

#define MAX_STRING_LENGTH 128

parameters_t parameters;

static const char *const param_names[] =
    {
        [ePARAM_OPCODE_WIDTH] = "opcode_width",
        [ePARAM_MEMORY_WIDTH] = "memory_width",
        [ePARAM_ALIGNMENT] = "alignment",
        [ePARAM_ADDRESS_WIDTH] = "address_width",
        [ePARAM_ADDRESS_START] = "address_start",
        [ePARAM_ADRRESS_STOP] = "address_stop",
        [ePARAM_ENDIANNESS] = "endianness",
        [ePARAM_ARGS_SEPARATOR] = "args_separator",
        [ePARAM_LABEL_PATTERN] = "label_pattern"
    };

typedef struct
{
    int pattern; //TODO: make this shit
} bit_pattern_t;

typedef struct
{
    char *pattern;
    bit_pattern_t byte_pattern;
} opcode_t;

linked_list_t *opcode_list;

int search_match(const char *const string_list[], int n_strings, const char *pattern);

int command_param(linked_list_t *args)
{
    if (!args)
        return -1; //No arguments

    linked_list_t *first_arg = list_get_at(args, 0); //Get the name
    int len = list_get_lenght(args);

    if (first_arg == NULL)
        return -2; //Argument 0 (param name) is empty

    if (first_arg->data_type != eDATA)
        return -3; //List element is not a data object

    //Get the parameter number
    char *param_name = ((data_t *)(first_arg->user_data))->strVal;
    int param_number = search_match(param_names, ePARAM_len, param_name);

    //Check if it is a correct parameter
    if (param_number < 0)
        return -4; //Unkown parameter

    linked_list_t* arg_array[len]; //Flexible array
    for(int i = 1; i < len; i++)
    {
        arg_array[i - 1] = list_get_at(args, i); //Innefficient, but really I don't care
    }

    switch (param_number)
    {
    case ePARAM_OPCODE_WIDTH:
        if (len != 2)
            return -5; //Bad arguments

        arg_array[0] = list_get_at(args, 1);
        if (arg_array[0]->type != T_INTEGER)
            return -5; // Bad argument

        if (parameters.opcode_width > 0)
            return 1; //Warning, redeclaring opcode_width

        parameters.opcode_width = ((data_t *)(arg_array[0]->user_data))->iVal;
        return 0; //Success
        break;

    case ePARAM_MEMORY_WIDTH:
        if (len != 2)
            return -5; //Bad arguments

        arg_array[0] = list_get_at(args, 1);
        if (arg_array[0]->type != T_INTEGER)
            return -5; // Bad argument

        if (parameters.memory_width > 0)
            return 1; //Warning, redeclaring opcode_width

        parameters.memory_width = ((data_t *)(arg_array[0]->user_data))->iVal;
        return 0; //Success
        break;

    case ePARAM_ALIGNMENT:
        if (len != 2)
            return -5; //Bad arguments

        arg_array[0] = list_get_at(args, 1);
        if (arg_array[0]->type != T_INTEGER)
            return -5; // Bad argument

        if (parameters.alignment > 0)
            return 1; //Warning, redeclaring opcode_width

        parameters.alignment = ((data_t *)(arg_array[0]->user_data))->iVal;
        return 0; //Success
        break;

    case ePARAM_ADDRESS_WIDTH:
        if (len != 2)
            return -5; //Bad arguments

        arg_array[0] = list_get_at(args, 1);
        if (arg_array[0]->type != T_INTEGER)
            return -5; // Bad argument

        if (parameters.address_width > 0)
            return 1; //Warning, redeclaring opcode_width

        parameters.address_width = ((data_t *)(arg_array[0]->user_data))->iVal;
        return 0; //Success
        break;

    case ePARAM_ADDRESS_START:
        if (len != 2)
            return -5; //Bad arguments

        arg_array[0] = list_get_at(args, 1);
        if (arg_array[0]->type != T_INTEGER)
            return -5; // Bad argument

        if (parameters.address_start > 0)
            return 1; //Warning, redeclaring opcode_width

        parameters.address_start = ((data_t *)(arg_array[0]->user_data))->iVal;
        return 0; //Success
        break;

    case ePARAM_ADRRESS_STOP:
        if (len != 2)
            return -5; //Bad arguments

        arg_array[0] = list_get_at(args, 1);
        if (arg_array[0]->type != T_INTEGER)
            return -5; // Bad argument

        if (parameters.address_stop > 0)
            return 1; //Warning, redeclaring opcode_width

        parameters.address_stop = ((data_t *)(arg_array[0]->user_data))->iVal;
        return 0; //Success
        break;

    case ePARAM_ENDIANNESS:
        if (len != 2)
            return -5; //Bad arguments

        arg_array[0] = list_get_at(args, 1);
        if (arg_array[0]->type != T_STRING)
            return -5; // Bad argument

        if (parameters.endianness > 0)
            return 1; //Warning, redeclaring opcode_width

        char* str = ((data_t *)(arg_array[0]->user_data))->strVal;

        if(strcmp(str, "little") == 0)
        {
            parameters.endianness = eLITTLE_ENDIAN;
        }
        else if(strcmp(str, "big") == 0)
        {
            parameters.endianness = eBIG_ENDIAN;
        }
        else
        {
            parameters.endianness = eUNDEF_ENDIAN;
            return -1;
        }

        return 0; //Success
        break;

    case ePARAM_ARGS_SEPARATOR:
        if (len != 2)
            return -5; //Bad arguments

        arg_array[0] = list_get_at(args, 1);
        if (arg_array[0]->type != T_STRING)
            return -5; // Bad argument

        if (parameters.args_separator > 0)
            return 1; //Warning, redeclaring opcode_width

        parameters.args_separator = ((data_t *)(arg_array[0]->user_data))->strVal[0];
        return 0; //Success
        break;

    case ePARAM_LABEL_PATTERN:
        if (len != 2)
            return -5; //Bad arguments

        arg_array[0] = list_get_at(args, 1);
        if (arg_array[0]->type != T_STRING)
            return -5; // Bad argument

        if (parameters.label_pattern > 0)
            return 1; //Warning, redeclaring opcode_width

        parameters.label_pattern = ((data_t *)(arg_array[0]->user_data))->strVal;
        return 0; //Success
        break;

    default:
        abort();
        break;
    }
}

int search_match(const char *const string_list[], int n_strings, const char *pattern)
{
    for (int i = 0; i < n_strings; i++)
    {
        if (0 == strncmp(string_list[i], pattern, MAX_STRING_LENGTH))
            return i; //Got a match
    }

    return -1; //No match
}