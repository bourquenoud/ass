#include "parameters.h"

#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "ast_node.h"
#include "generated/ass.tab.h"
#include "linked_list.h"
#include "hash_array.h"
#include "constants.h"

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
        [ePARAM_LABEL_PATTERN] = "label_pattern"};

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
bool try_get_integer(linked_list_t *, int64_t *);
bool try_get_string(linked_list_t *, char**);
bool try_get_string(linked_list_t *, char**);

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

    linked_list_t *arg_array[len]; //Flexible array
    for (int i = 1; i < len; i++)
    {
        arg_array[i - 1] = list_get_at(args, i); //Innefficient, but really I don't care
    }

    int64_t val;
    char* str;

    switch (param_number)
    {
    case ePARAM_OPCODE_WIDTH:
        if (len != 2)
            return -5; //Bad arguments

        arg_array[0] = list_get_at(args, 1);
        if (!try_get_integer(arg_array[0], &val))
            return -5;

        if (parameters.opcode_width > 0)
            return 1; //Warning, redeclaring opcode_width

        parameters.opcode_width = val;
        return 0; //Success
        break;

    case ePARAM_MEMORY_WIDTH:
        if (len != 2)
            return -5; //Bad arguments

        arg_array[0] = list_get_at(args, 1);
        if (!try_get_integer(arg_array[0], &val))
            return -5;

        if (parameters.memory_width > 0)
            return 1; //Warning, redeclaring opcode_width

        parameters.memory_width = val;
        return 0; //Success
        break;

    case ePARAM_ALIGNMENT:
        if (len != 2)
            return -5; //Bad arguments

        arg_array[0] = list_get_at(args, 1);
        if (!try_get_integer(arg_array[0], &val))
            return -5;

        if (parameters.alignment > 0)
            return 1; //Warning, redeclaring opcode_width

        parameters.alignment = val;
        return 0; //Success
        break;

    case ePARAM_ADDRESS_WIDTH:
        if (len != 2)
            return -5; //Bad arguments

        arg_array[0] = list_get_at(args, 1);
        if (!try_get_integer(arg_array[0], &val))
            return -5;

        if (parameters.address_width > 0)
            return 1; //Warning, redeclaring opcode_width

        parameters.address_width = val;
        return 0; //Success
        break;

    case ePARAM_ADDRESS_START:
        if (len != 2)
            return -5; //Bad arguments

        arg_array[0] = list_get_at(args, 1);
        if (!try_get_integer(arg_array[0], &val))
            return -5;

        if (parameters.address_start > 0)
            return 1; //Warning, redeclaring opcode_width

        parameters.address_start = val;
        return 0; //Success
        break;

    case ePARAM_ADRRESS_STOP:
        if (len != 2)
            return -5; //Bad arguments

        arg_array[0] = list_get_at(args, 1);
        if (!try_get_integer(arg_array[0], &val))
            return -5;

        if (parameters.address_stop > 0)
            return 1; //Warning, redeclaring opcode_width

        parameters.address_stop = val;
        return 0; //Success
        break;

    case ePARAM_ENDIANNESS:
        if (len != 2)
            return -5; //Bad arguments

        arg_array[0] = list_get_at(args, 1);
        if (!try_get_string(arg_array[0], &str))
            return -5;

        if (parameters.endianness > 0)
            return 1; //Warning, redeclaring opcode_width

        if (strcmp(str, "little") == 0)
        {
            parameters.endianness = eLITTLE_ENDIAN;
        }
        else if (strcmp(str, "big") == 0)
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
        if (!try_get_string(arg_array[0], &str))
            return -5;

        if (parameters.args_separator > 0)
            return 1; //Warning, redeclaring opcode_width

        parameters.args_separator = str[0];
        return 0; //Success
        break;

    case ePARAM_LABEL_PATTERN:
        if (len != 2)
            return -5; //Bad arguments

        arg_array[0] = list_get_at(args, 1);
        if (!try_get_string(arg_array[0], &str))
            return -5;

        if (parameters.label_pattern > 0)
            return 1; //Warning, redeclaring opcode_width

        parameters.label_pattern = str;
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

bool try_get_integer(linked_list_t *element, int64_t *result)
{
    if (element->type == T_INTEGER)
    {
        //Extract the data from the list element
        *result = ((data_t *)(element->user_data))->iVal;
        return true;
    }
    else if (element->type == T_IDENTIFIER)
    {
        //Extract the constant name from the list element
        char *key = ((data_t *)(element->user_data))->strVal;
        if (hash_check_key(int_const_array, key))
        {
            *result = ((data_t *)hash_get(int_const_array, key))->iVal;
            return true;
        }
        else
        {
            return false;
        }
    }
    return false;
}

bool try_get_string(linked_list_t *element, char** result)
{
    if (element->type == T_STRING)
    {
        //Extract the data from the list element
        *result = ((data_t *)(element->user_data))->strVal;
        return true;
    }
    else if (element->type == T_IDENTIFIER)
    {
        //Extract the constant name from the list element
        char *key = ((data_t *)(element->user_data))->strVal;
        if (hash_check_key(str_const_array, key))
        {
            *result = ((data_t *)hash_get(str_const_array, key))->strVal;
            return true;
        }
        else
        {
            return false;
        }
    }
    return false;
}