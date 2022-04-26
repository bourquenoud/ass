#include "commands.h"

#include <string.h>

#include "failure.h"
#include "xmalloc.h"
#include "macro.h"

#define TABLE_SIZE 1024

hash_t *bit_const_array;
hash_t *int_const_array;
hash_t *str_const_array;
hash_t *enum_array;
hash_t *format_array;
hash_t *macro_array;

darray_t *opcode_array;

darray_t *custom_output_array;

char *override_code[1] = {NULL};
char *code = NULL;

static void xmalloc_callback(int err);

void command_init()
{
    enum_array = hash_init(TABLE_SIZE);
    bit_const_array = hash_init(TABLE_SIZE);
    int_const_array = hash_init(TABLE_SIZE);
    str_const_array = hash_init(TABLE_SIZE);
    format_array = hash_init(TABLE_SIZE);
    macro_array = hash_init(TABLE_SIZE);
    opcode_array = darray_init(sizeof(opcode_t));
    custom_output_array = darray_init(sizeof(custom_output_t));
}

void check_any(char *key)
{
    if (hash_check_key(bit_const_array, key) | hash_check_key(int_const_array, key) | hash_check_key(str_const_array, key))
        fail_error("'%s' already declared as a constant. You can not declare an enum or a constant twice.", key);
    if (hash_check_key(enum_array, key))
        fail_error("'%s'  already declared as an enum. You can not declare an enum or a constant twice.", key);
    if (hash_check_key(format_array, key))
        fail_error("'%s'  already declared as an opcode format.", key);
    if (hash_check_key(macro_array, key))
        fail_error("'%s'  already declared as a macro.", key);
}

int command_bit_const(data_t *id, data_t *value)
{
    // Check if the constant already exists
    check_any(id->strVal);

    bit_const_t *data = &(value->bVal);

    hash_add(bit_const_array, id->strVal, (void *)data);
    return 0; // Success
}

int command_int_const(data_t *id, data_t *value)
{
    // Check if the constant already exists
    check_any(id->strVal);

    hash_add(int_const_array, id->strVal, (void *)value);
    return 0; // Success
}

int command_str_const(data_t *id, data_t *value)
{
    // Check if the constant already exists
    check_any(id->strVal);

    hash_add(str_const_array, id->strVal, (void *)value);
    return 0; // Success
}

int command_enum(data_t *id, data_t *value)
{
    // Check if the constant already exists
    check_any(id->strVal);

    int len = strlen(id->strVal) + 1; //+1 to include the NULL

    xmalloc_set_handler(xmalloc_callback);
    enumeration_t *new_enumeration = xmalloc(sizeof(enumeration_t) + len);
    new_enumeration->width = value->iVal;
    new_enumeration->pattern_list = NULL;
    strcpy(new_enumeration->name, id->strVal);

    hash_add(enum_array, id->strVal, (void *)new_enumeration);
    return 0; // Success
}

int command_format(data_t *id, linked_list_t *list)
{
    fail_debug("Computing opcode format \"%s\"", id->strVal);

    // Check if the constant already exists
    check_any(id->strVal);

    // Get the opcode width parameter
    int expected_width = parameters.opcode_width;
    if (expected_width < 0)
    {
        fail_error("Opcode width not set.");
        return 1;
    }

    // Number the elements in order, compute the width and extract the ellipsis
    bool has_id = false;
    int width = 0;
    linked_list_t *current = list;
    int index_opcode = 0;
    int index_mnemonic = 0;
    bit_elem_t *ellipsis = NULL;
    while (current != NULL)
    {
        width += ((bit_elem_t *)(current->user_data))->width;
        // Skip non-arguments
        if (((bit_elem_t *)(current->user_data))->index_mnemonic >= 0)
        {
            ((bit_elem_t *)(current->user_data))->index_mnemonic = index_mnemonic;
            index_mnemonic++;
        }
        ((bit_elem_t *)(current->user_data))->index_opcode = index_opcode;
        index_opcode++;

        // Check for multiple ellipsis
        if (((bit_elem_t *)(current->user_data))->type == eBP_ELLIPSIS)
        {
            if (ellipsis != NULL)
            {
                fail_error("Multiple ellipsis (...) in a bit format.");
                return 1;
            }
            ellipsis = ((bit_elem_t *)(current->user_data));
        }
        // Check for multiple ID
        else if (((bit_elem_t *)(current->user_data))->type == eBP_ID)
        {
            if (has_id)
            {
                fail_error("Multiple ID substitutions in a bit format.");
                return 1;
            }
            has_id = true;
        }

        //Log a details message specifing the width and the type of the element
        fail_debug("Element %d: %d bits, type %s", index_opcode, ((bit_elem_t*)(current->user_data))->width,
                    name_BPTYPE[((bit_elem_t*)(current->user_data))->type]);

        current = current->next;
    }

    // Check if the format width is correct, and compute the ellipsis width
    if (width > expected_width)
    {
        fail_error("Format '%s' width (%i) is larger than the expected one (%i).", id->strVal, width, expected_width);
        return 1;
    }
    else if (width == expected_width)
    {
        if (ellipsis != NULL)
            fail_info("Ellipsis is useless in '%s'", id->strVal);
    }
    else
    {
        // Set the ellipsis size
        if (ellipsis != NULL)
        {
            //Log the ellipsis width
            fail_debug("Ellipsis width: %i", expected_width - width);
            ellipsis->width = expected_width - width;
        }
        else
        {
            fail_error("Format '%s' width (%i) is smaller than the expected one (%i).", id->strVal, width, expected_width);
            return 1;
        }
    }

    hash_add(format_array, id->strVal, (void *)list);
    return 0; // Success
}

int command_order(data_t *id, linked_list_t *order_args)
{
    if (!hash_check_key(format_array, id->strVal))
    {
        fail_error("The opcode format '%s' has not been declared.", id->strVal);
        return 1;
    }

    // Count the number of argument we want to reorder
    int n_reorder = 0;
    linked_list_t *current = order_args;
    while (current != NULL)
    {
        n_reorder++;
        current = current->next;
    }

    linked_list_t *start_bit_format_list = (linked_list_t *)hash_get(format_array, id->strVal);
    linked_list_t *bit_format_list = start_bit_format_list;

    // Count the number of arguments for the mnemonic
    int n_args = 0;
    while (bit_format_list != NULL)
    {
        if (((bit_elem_t *)(bit_format_list->user_data))->index_mnemonic >= 0)
            n_args++;

        bit_format_list = bit_format_list->next;
    }

    // Check if we are reordering the correct number of arguments
    if (n_reorder != n_args)
    {
        fail_error(
            "%s arguments, '%s' have %i arguments but got %i.",
            (n_reorder >= n_args) ? "Too many" : "Not enough",
            id->strVal,
            n_args,
            n_reorder);
        return -1;
    }

    int index = 0;
    while (order_args != NULL)
    {
        bit_format_list = start_bit_format_list;
        while (bit_format_list != NULL)
        {
            bit_elem_t *bit_elem = (bit_elem_t *)(bit_format_list->user_data);
            if (bit_elem->index_opcode == ((data_t *)(order_args->user_data))->iVal)
            {
                if (bit_elem->index_mnemonic < 0)
                {
                    fail_error("Trying to reorder a non-argument (index %i).", bit_elem->index_opcode);
                    return -1;
                }
                bit_elem->index_mnemonic = index;
                n_args--;
                break;
            }

            bit_format_list = bit_format_list->next;
        }
        if (bit_format_list == NULL)
            fail_error("Reordering out-of-range (index %i).", ((data_t *)(order_args->user_data))->iVal);
        index++;
        order_args = order_args->next;
    }

    return 0;
}

int command_opcode(data_t *id, data_t *pattern, data_t *opcode_id, bool is_constant)
{
    // Check for common errors
    if (!hash_check_key(format_array, id->strVal))
    {
        fail_error("The opcode format '%s' has not been declared.", id->strVal);
        return 1;
    }
    if (is_constant && !hash_check_key(bit_const_array, opcode_id->strVal))
    {
        fail_error("The bit constant '%s' has not been declared.", opcode_id->strVal);
        return 1;
    }
    int len = strlen(pattern->strVal) + 1; // +1 to count the terminating NULL
    if (len <= 1)
    {
        fail_error("Empty pattern.");
        return 1;
    }

    // Resolve the opcode id if necessary
    bit_const_t *resolved_opcode_id;
    if (is_constant)
        resolved_opcode_id = (bit_const_t *)hash_get(bit_const_array, opcode_id->strVal);
    else
        resolved_opcode_id = &(opcode_id->bVal);

    // Get the format
    linked_list_t *format_list = (linked_list_t *)hash_get(format_array, id->strVal);
    linked_list_t *current = format_list;

    // Check if the format expects an id
    bit_elem_t *id_bit_elem = NULL;
    bool require_id = false;
    while (current != NULL)
    {
        if (((bit_elem_t *)(current->user_data))->type == eBP_ID)
        {
            id_bit_elem = (bit_elem_t *)(current->user_data);
            require_id = true;
            break;
        }
        current = current->next;
    }

    // Produce warning if the opcode ID is not the correct width
    if (opcode_id != NULL)
    {
        if (resolved_opcode_id->width < id_bit_elem->width)
        {
            fail_info(
                "The width of the id of the opcode \"%s\" is smaller than the expected width. Automatically zero-padded to %i bits.",
                pattern->strVal,
                id_bit_elem->width);
            resolved_opcode_id->width = id_bit_elem->width;
        }
        else if (resolved_opcode_id->width > id_bit_elem->width && (resolved_opcode_id->width & id_bit_elem->width) == 0)
        {
            fail_info(
                "The width of the id of the opcode \"%s\" is greater than the expected width. Automatically trucated to %i bits.",
                pattern->strVal,
                id_bit_elem->width);
            resolved_opcode_id->width = id_bit_elem->width;
        }
        else if(resolved_opcode_id->width > id_bit_elem->width && (resolved_opcode_id->width & id_bit_elem->width) != 0)
        {
            fail_warning(
                "The width of the id of the opcode \"%s\" is greater than the expected width. Automatically trucated to %i bits. Some bits have been lost.",
                pattern->strVal,
                id_bit_elem->width);
            resolved_opcode_id->width = id_bit_elem->width;
        }
    }

    // Error if we expect an id but don't receive one
    if (opcode_id == NULL && require_id)
    {
        fail_error("'%s' expects an opcode id, but none given.", id->strVal);
        return 1;
    }

    // Warning if we receive an useless id
    if (opcode_id != NULL && !require_id)
        fail_warning("'%s' does not expect an opcode id. Ignored.", id->strVal);

    // Create a new opcode and clone the bitpattern
    opcode_t new_opcode;
    new_opcode.text_pattern = pattern->strVal;
    new_opcode.bit_pattern = NULL;
    linked_list_t *list_element;
    current = format_list;
    while (current != NULL)
    {
        if (((bit_elem_t *)(current->user_data))->type == eBP_ID) // Replace the id
        {
            bit_elem_t *new_bit_elem = bit_elem_init(
                eBP_BIT_LIT,
                ((bit_elem_t *)(current->user_data))->width,
                resolved_opcode_id);
            new_bit_elem->index_mnemonic = -1;
            new_bit_elem->index_opcode = ((bit_elem_t *)(current->user_data))->index_opcode;
            list_element = list_init(eBP_BIT_LIT, (void *)new_bit_elem, current->data_type);
        }
        else
            list_element = list_init(current->type, current->user_data, current->data_type);

        if (new_opcode.bit_pattern == NULL)
            new_opcode.bit_pattern = list_element;
        else
            list_append(new_opcode.bit_pattern, list_element);

        current = current->next;
    }

    darray_add(&opcode_array, new_opcode);

    return 0;
}

int command_pattern(data_t *enum_id, data_t *pattern_data, data_t *bit_const_data)
{
    if (!hash_check_key(enum_array, enum_id->strVal))
    {
        fail_error("The enum '%s' has not been declared.", enum_id->strVal);
        return 1;
    }

    int len = strlen(pattern_data->strVal) + 1; // +1 to count the terminating NULL

    if (len <= 1)
    {
        fail_error("Empty pattern.");
        return 1;
    }

    xmalloc_set_handler(xmalloc_callback);
    enumeration_t *enumeration = (enumeration_t *)hash_get(enum_array, enum_id->strVal);
    pattern_t *pattern = xmalloc(sizeof(pattern_t) + sizeof(char[len]));
    pattern->bit_const = bit_const_data->bVal;
    strcpy(pattern->pattern, pattern_data->strVal);

    if (pattern->bit_const.width < enumeration->width)
    {
        fail_info("The pattern \"%s\" width is smaller then the enum '%s' width."
                  " Automatically zero-padded to %i bits.",
                  pattern->pattern,
                  enumeration->name,
                  enumeration->width);
        pattern->bit_const.width = enumeration->width; // Force the bit_constant to have the correct width
    }

    if (pattern->bit_const.width > enumeration->width)
    {
        fail_warning("The pattern \"%s\" width is greater then the enum '%s' width."
                     " Automatically trucated to %i bits.",
                     pattern->pattern,
                     enumeration->name,
                     enumeration->width);
        pattern->bit_const.width = enumeration->width; // Force the bit_constant to have the correct width
    }

    // Create the list or append it
    if (enumeration->pattern_list == NULL)
        enumeration->pattern_list = list_init(0, (void *)pattern, eDATA);
    else
        list_append(enumeration->pattern_list, list_init(0, (void *)pattern, eDATA));
}

int command_code(data_t *in_code)
{
    // Copy the code if first command invocation
    if (code == NULL)
    {
        xmalloc_set_handler(xmalloc_callback);
        code = xmalloc(strlen(in_code->strVal) + 1);
        strcpy(code, in_code->strVal);
    }
    // Otherwise append it
    else
    {
        code = realloc(code, strlen(code) + strlen(in_code->strVal) + 1);
        strcat(code, in_code->strVal);
    }
    return 0;
}

int command_output(data_t *name, data_t *description, data_t *in_code)
{
    custom_output_t new_custom_output = {
        .name = name->strVal,
        .description = description->strVal,
        .code = in_code->strVal,
    };

    // Check if it already exists
    custom_output_t* array = darray_get_ptr(&custom_output_array, 0);
    for(int i = 0; i < custom_output_array->count; i++)
    {
        if(strcmp(array[i].name, new_custom_output.name) == 0)
        {
            fail_error("'%s' is already defined as a custom output type.");
            return 1;
        }
    }

    darray_add(&custom_output_array, new_custom_output);
    return 0;
}

int command_override(data_t *target_name, data_t *in_code)
{
    if (strcmp(target_name->strVal, "startup") != 0)
    {
        fail_error("Unknown target '%s'", target_name->strVal);
        return 1;
    }

    // Copy the code if first command invocation
    if (override_code[0] == NULL)
    {
        // Allocate the memory
        xmalloc_set_handler(xmalloc_callback);
        override_code[0] = xmalloc(strlen(in_code->strVal) + 1);
        strcpy(override_code[0], in_code->strVal);
    }
    // Otherwise append it
    else
    {
        // Realloc the code
        override_code[0] = realloc(override_code[0], strlen(override_code[0]) + strlen(in_code->strVal) + 1);
        strcat(override_code[0], in_code->strVal);
    }
}

int command_macro(data_t *name, data_t *content)
{
    // Check if it already exists in any of the arrays
    check_any(name->strVal);

    // Create the macro
    xmalloc_set_handler(xmalloc_callback);
    macro_t* new_macro = xmalloc(sizeof(macro_t));
    new_macro->name = name->strVal;
    new_macro->content = content->strVal;

    // Add it to the array
    hash_add(macro_array, name->strVal, (void *)new_macro);
    return 0;
}

/********************************************************/

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