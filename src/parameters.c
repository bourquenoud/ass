#include "parameters.h"

#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "failure.h"
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
        [ePARAM_LABEL_PATTERN] = "label_pattern",
};

int search_match(const char *const string_list[], int n_strings, const char *pattern);
bool try_get_integer(linked_list_t *, int64_t *);
bool try_get_string(linked_list_t *, char **);
bool try_get_string(linked_list_t *, char **);

int command_param(linked_list_t *args)
{
    if (!args)
        fail_error("No parameter name found.");

    linked_list_t *first_arg = list_get_at(args, 0); //Get the name
    int len = list_get_lenght(args);

    if (first_arg == NULL)
        fail_error("No parameter name found.");

    if (first_arg->data_type != eDATA)
        fail_error("Critical parsing error.");

    //Get the parameter number
    char *param_name = ((data_t *)(first_arg->user_data))->strVal;
    int param_number = search_match(param_names, ePARAM_len, param_name);

    //Check if it is a correct parameter
    if (param_number < 0)
        fail_error("Unkown parameter '%s'", param_name);

    linked_list_t *arg_array[len]; //Flexible array
    for (int i = 1; i < len; i++)
    {
        arg_array[i - 1] = list_get_at(args, i); //Innefficient, but really I don't care
    }

    int64_t val;
    char *str;

    switch (param_number)
    {
    case ePARAM_OPCODE_WIDTH:
        if (len != 2)
            fail_error("Parameter '%s' expects one integer.", param_name);

        arg_array[0] = list_get_at(args, 1);
        if (!try_get_integer(arg_array[0], &val))
            fail_error("Parameter '%s' expects one integer.", param_name);

        if (parameters.opcode_width > 0)
            fail_warning("Parameter '%s' set more than once.", param_name);

        parameters.opcode_width = val;
        return 0; //Success
        break;

    case ePARAM_MEMORY_WIDTH:
        if (len != 2)
            fail_error("Parameter '%s' expects one integer.", param_name);

        arg_array[0] = list_get_at(args, 1);
        if (!try_get_integer(arg_array[0], &val))
            fail_error("Parameter '%s' expects one integer.", param_name);

        if (parameters.memory_width > 0)
            fail_warning("Parameter '%s' set more than once.", param_name);

        parameters.memory_width = val;
        return 0; //Success
        break;

    case ePARAM_ALIGNMENT:
        if (len != 2)
            fail_error("Parameter '%s' expects one integer.", param_name);

        arg_array[0] = list_get_at(args, 1);
        if (!try_get_integer(arg_array[0], &val))
            fail_error("Parameter '%s' expects one integer.", param_name);

        if (parameters.alignment > 0)
            fail_warning("Parameter '%s' set more than once.", param_name);

        parameters.alignment = val;
        return 0; //Success
        break;

    case ePARAM_ADDRESS_WIDTH:
        if (len != 2)
            fail_error("Parameter '%s' expects one integer.", param_name);

        arg_array[0] = list_get_at(args, 1);
        if (!try_get_integer(arg_array[0], &val))
            fail_error("Parameter '%s' expects one integer.", param_name);

        if (parameters.address_width > 0)
            fail_warning("Parameter '%s' set more than once.", param_name);

        parameters.address_width = val;
        return 0; //Success
        break;

    case ePARAM_ADDRESS_START:
        if (len != 2)
            fail_error("Parameter '%s' expects one integer.", param_name);

        arg_array[0] = list_get_at(args, 1);
        if (!try_get_integer(arg_array[0], &val))
            fail_error("Parameter '%s' expects one integer.", param_name);

        if (parameters.address_start > 0)
            fail_warning("Parameter '%s' set more than once.", param_name);

        parameters.address_start = val;
        return 0; //Success
        break;

    case ePARAM_ADRRESS_STOP:
        if (len != 2)
            fail_error("Parameter '%s' expects one integer.", param_name);

        arg_array[0] = list_get_at(args, 1);
        if (!try_get_integer(arg_array[0], &val))
            fail_error("Parameter '%s' expects one integer.", param_name);

        if (parameters.address_stop > 0)
            fail_warning("Parameter '%s' set more than once.", param_name);

        parameters.address_stop = val;
        return 0; //Success
        break;

    case ePARAM_ENDIANNESS:
        if (len != 2)
            fail_error("Parameter '%s' expects one string.", param_name);

        arg_array[0] = list_get_at(args, 1);
        if (!try_get_string(arg_array[0], &str))
            fail_error("Parameter '%s' expects one string.", param_name);

        if (parameters.endianness > 0)
            fail_warning("Parameter '%s' set more than once.", param_name);

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
            fail_error("Parameter '%s' expects either \"big\" or \"little\"", param_name);
        }

        return 0; //Success
        break;

    case ePARAM_ARGS_SEPARATOR:
        if (len != 2)
            fail_error("Parameter '%s' expects one string.", param_name);

        arg_array[0] = list_get_at(args, 1);
        if (!try_get_string(arg_array[0], &str))
            fail_error("Parameter '%s' expects one string.", param_name);

        if (parameters.args_separator > 0)
            fail_warning("Parameter '%s' set more than once.", param_name);

        parameters.args_separator = str[0];
        return 0; //Success
        break;

    case ePARAM_LABEL_PATTERN:
        if (len != 2)
            fail_error("Parameter '%s' expects one string.", param_name);

        arg_array[0] = list_get_at(args, 1);
        if (!try_get_string(arg_array[0], &str))
            fail_error("Parameter '%s' expects one string.", param_name);

        if (parameters.label_pattern > 0)
            fail_warning("Parameter '%s' set more than once.", param_name);

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

bool try_get_string(linked_list_t *element, char **result)
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