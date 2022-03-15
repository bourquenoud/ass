#include "parameters.h"

#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "failure.h"
#include "ast_node.h"
#include "generated/ass.tab.h"
#include "linked_list.h"
#include "hash_array.h"
#include "commands.h"

#define MAX_STRING_LENGTH 128

parameters_t parameters;

/**
 * @brief Parameters set before parsing the file.
 *
 */
const parameters_t start_parameters =
    {
        // No default value, throw an error if not defined
        .opcode_width = -1,
        .memory_width = -1,
        .alignment = -1,
        .address_width = -1,
        .address_start = -1,
        .address_stop = -1,
        .endianness = eUNDEF_ENDIAN,
        .args_separator = '\0',
        .label_postfix = '\0',
        .constant_dir = NULL,
        .author = NULL,
        .version = NULL,
        .name = NULL,
        .copyright = NULL,
        .description = NULL,
};

/**
 * @brief Default parameters. Values not set in the file are replace after parsing by those values.
 *
 */
const parameters_t default_parameters =
    {
        // No default value, throw an error if not defined
        .opcode_width = -1,
        .memory_width = -1,
        .alignment = -1,
        .address_width = -1,
        .address_start = -1,
        .address_stop = -1,
        .endianness = eUNDEF_ENDIAN,

        // Default values
        .args_separator = ',',
        .label_postfix = ':',
        .constant_dir = "\\.constant",
        .author = "[NOT SET]",
        .version = "[NOT SET]",
        .name = "[NOT SET]",
        .copyright = "[NOT SET]",
        .description = "Assembler generated using the Assembly Syntax Sythesiser."
};

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
        [ePARAM_LABEL_POSTFIX] = "label_postfix",
        [ePARAM_CONSTANT_DIRECTIVE] = "constant_directive",
        [ePARAM_AUTHOR] = "author",
        [ePARAM_NAME] = "name",
        [ePARAM_VERSION] = "version",
        [ePARAM_COPYRIGHT] = "copyright",
        [ePARAM_DESCRIPTION] = "description",
};

int search_match(const char *const string_list[], int n_strings, const char *pattern);
bool try_get_integer(linked_list_t *, int64_t *);
bool try_get_string(linked_list_t *, char **);
bool try_get_string(linked_list_t *, char **);

void param_init()
{
    parameters = start_parameters;
}

void param_fill_unset()
{
    // TODO: set an error location
    if (parameters.opcode_width < 0)
        fail_error("opcode_width never set");
    if (parameters.opcode_width < 0)
        fail_error("opcode_width never set");
    if (parameters.memory_width < 0)
        fail_error("memory_width never set");
    if (parameters.alignment < 0)
        fail_error("alignment never set");
    if (parameters.address_width < 0)
        fail_error("address_width never set");
    if (parameters.address_start < 0)
        fail_error("address_start never set");
    if (parameters.address_stop < 0)
        fail_error("address_stop never set");
    if (parameters.endianness < 0)
        fail_error("endianness never set");

    // Set default values
    if (parameters.args_separator == '\0')
    {
        fail_info("args_separator never set, falling back to default ('%c')", default_parameters.args_separator);
        parameters.args_separator = default_parameters.args_separator;
    }
    if (parameters.label_postfix == '\0')
    {
        fail_info("label_postfix never set, falling back to default ('%c')", default_parameters.label_postfix);
        parameters.label_postfix = default_parameters.label_postfix;
    }
    if (parameters.constant_dir == NULL)
    {
        fail_info("constant_directive never set, falling back to default ('%s')", default_parameters.constant_dir);
        parameters.constant_dir = default_parameters.constant_dir;
    }
    if (parameters.author == NULL)
    {
        fail_info("Author never set, falling back to default ('%s')", default_parameters.author);
        parameters.author = default_parameters.author;
    }
    if (parameters.version == NULL)
    {
        fail_info("Version never set, falling back to default ('%s')", default_parameters.version);
        parameters.version = default_parameters.version;
    }
    if (parameters.name == NULL)
    {
        fail_info("Name never set, falling back to default ('%s')", default_parameters.name);
        parameters.name = default_parameters.name;
    }
    if (parameters.copyright == NULL)
    {
        fail_info("Copyright never set, falling back to default ('%s')", default_parameters.copyright);
        parameters.copyright = default_parameters.copyright;
    }
    if (parameters.description == NULL)
    {
        fail_info("Description never set, falling back to default ('%s')", default_parameters.description);
        parameters.description = default_parameters.description;
    }
}

int command_param(linked_list_t *args)
{

    if (!args)
        fail_error("No parameter name found.");

    linked_list_t *first_arg = list_get_at(args, 0); // Get the name
    int len = list_get_lenght(args);

    if (first_arg == NULL)
        fail_error("No parameter name found.");

    if (first_arg->data_type != eDATA)
        fail_error("Critical parsing error.");

    // Get the parameter number
    char *param_name = ((data_t *)(first_arg->user_data))->strVal;
    int param_number = search_match(param_names, ePARAM_len, param_name);

    // Check if it is a correct parameter
    if (param_number < 0)
    {
        fail_error("Unkown parameter '%s'", param_name);
        return 1;
    }

    linked_list_t *arg_array[len]; // Flexible array
    for (int i = 1; i < len; i++)
    {
        arg_array[i - 1] = list_get_at(args, i); // Innefficient, but really I don't care
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
        return 0; // Success
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
        return 0; // Success
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
        return 0; // Success
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
        return 0; // Success
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
        return 0; // Success
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
        return 0; // Success
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

        return 0; // Success
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
        return 0; // Success
        break;

    case ePARAM_LABEL_POSTFIX:
        if (len != 2)
            fail_error("Parameter '%s' expects one string.", param_name);

        arg_array[0] = list_get_at(args, 1);
        if (!try_get_string(arg_array[0], &str))
            fail_error("Parameter '%s' expects one string.", param_name);

        if (parameters.label_postfix > 0)
            fail_warning("Parameter '%s' set more than once.", param_name);

        if (strlen(str) != 1)
            fail_warning("Parameter '%s' expectect a string of exactly on character", param_name);

        parameters.label_postfix = str[0];
        return 0; // Success
        break;

    case ePARAM_CONSTANT_DIRECTIVE:
        if (len != 2)
            fail_error("Parameter '%s' expects one string.", param_name);

        arg_array[0] = list_get_at(args, 1);
        if (!try_get_string(arg_array[0], &str))
            fail_error("Parameter '%s' expects one string.", param_name);

        if (parameters.constant_dir != NULL)
            fail_warning("Parameter '%s' set more than once.", param_name);

        if (strlen(str) <= 0)
        {
            fail_error("Constant directive is too short.");
            return 1; // Error
        }

        parameters.constant_dir = str; // Passed by ref, may need to be copied for stability
        return 0;                      // Success
        break;

    case ePARAM_AUTHOR:
        if (len != 2)
            fail_error("Parameter '%s' expects one string.", param_name);

        arg_array[0] = list_get_at(args, 1);
        if (!try_get_string(arg_array[0], &str))
            fail_error("Parameter '%s' expects one string.", param_name);

        if (parameters.constant_dir != NULL)
            fail_warning("Parameter '%s' set more than once.", param_name);

        parameters.author = str; // Passed by ref, may need to be copied for stability
        return 0;                // Success
        break;

    case ePARAM_VERSION:
        if (len != 2)
            fail_error("Parameter '%s' expects one string.", param_name);

        arg_array[0] = list_get_at(args, 1);
        if (!try_get_string(arg_array[0], &str))
            fail_error("Parameter '%s' expects one string.", param_name);

        if (parameters.constant_dir != NULL)
            fail_warning("Parameter '%s' set more than once.", param_name);

        parameters.version = str; // Passed by ref, may need to be copied for stability
        return 0;                 // Success
        break;

    case ePARAM_NAME:
        if (len != 2)
            fail_error("Parameter '%s' expects one string.", param_name);

        arg_array[0] = list_get_at(args, 1);
        if (!try_get_string(arg_array[0], &str))
            fail_error("Parameter '%s' expects one string.", param_name);

        if (parameters.constant_dir != NULL)
            fail_warning("Parameter '%s' set more than once.", param_name);

        parameters.name = str; // Passed by ref, may need to be copied for stability
        return 0;              // Success
        break;

    case ePARAM_COPYRIGHT:
        if (len != 2)
            fail_error("Parameter '%s' expects one string.", param_name);

        arg_array[0] = list_get_at(args, 1);
        if (!try_get_string(arg_array[0], &str))
            fail_error("Parameter '%s' expects one string.", param_name);

        if (parameters.constant_dir != NULL)
            fail_warning("Parameter '%s' set more than once.", param_name);

        parameters.copyright = str; // Passed by ref, may need to be copied for stability
        return 0;                   // Success
        break;

    case ePARAM_DESCRIPTION:
        if (len != 2)
            fail_error("Parameter '%s' expects one string.", param_name);

        arg_array[0] = list_get_at(args, 1);
        if (!try_get_string(arg_array[0], &str))
            fail_error("Parameter '%s' expects one string.", param_name);

        if (parameters.constant_dir != NULL)
            fail_warning("Parameter '%s' set more than once.", param_name);

        parameters.description = str; // Passed by ref, may need to be copied for stability
        return 0;                   // Success
        break;

    default:
        fail_error("Unimplemented parameter, please report to https://github.com/bourquenoud/ass/issues");
        abort();
        break;
    }
}

int search_match(const char *const string_list[], int n_strings, const char *pattern)
{
    for (int i = 0; i < n_strings; i++)
    {
        if (0 == strncmp(string_list[i], pattern, MAX_STRING_LENGTH))
            return i; // Got a match
    }

    return -1; // No match
}

bool try_get_integer(linked_list_t *element, int64_t *result)
{
    if (element->type == T_INTEGER)
    {
        // Extract the data from the list element
        *result = ((data_t *)(element->user_data))->iVal;
        return true;
    }
    else if (element->type == T_IDENTIFIER)
    {
        // Extract the constant name from the list element
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
        // Extract the data from the list element
        *result = ((data_t *)(element->user_data))->strVal;
        return true;
    }
    else if (element->type == T_IDENTIFIER)
    {
        // Extract the constant name from the list element
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