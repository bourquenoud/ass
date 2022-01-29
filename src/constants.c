#include "constants.h"

#include <string.h>

#include "failure.h"
#include "xmalloc.h"
#include "macro.h"

hash_t *bit_const_array;
hash_t *int_const_array;
hash_t *str_const_array;
hash_t *enum_array;

static void xmalloc_callback(int err);

void check_any(char *key)
{
    if (hash_check_key(bit_const_array, key) | hash_check_key(int_const_array, key) | hash_check_key(str_const_array, key))
        fail_error("'%s' already declared as a constant. You can not declare an enum or a constant twice.", key);
    if (hash_check_key(enum_array, key))
        fail_error("'%s'  already declared as an enum. You can not declare an enum or a constant twice.", key);
}

int command_bit_const(data_t *id, data_t *value)
{
    //Check if the constant already exists
    check_any(id->strVal);

    bit_const_t *data = &(value->bVal);

    hash_add(bit_const_array, id->strVal, (void *)data);
    return 0; //Success
}

int command_int_const(data_t *id, data_t *value)
{
    //Check if the constant already exists
    check_any(id->strVal);

    hash_add(int_const_array, id->strVal, (void *)value);
    return 0; //Success
}

int command_str_const(data_t *id, data_t *value)
{
    //Check if the constant already exists
    check_any(id->strVal);

    hash_add(str_const_array, id->strVal, (void *)value);
    return 0; //Success
}

int command_enum(data_t *id, data_t *value)
{
    //Check if the constant already exists
    check_any(id->strVal);

    int len = strlen(id->strVal) + 1; //+1 to include the NULL

    xmalloc_set_handler(xmalloc_callback);
    enumeration_t *new_enumeration = xmalloc(sizeof(enumeration_t) + sizeof(char[len]));
    new_enumeration->width = value->iVal;
    new_enumeration->pattern_list = NULL;
    strcpy(new_enumeration->name, id->strVal);

    hash_add(enum_array, id->strVal, (void *)new_enumeration);
    return 0; //Success
}

int command_pattern(data_t *enum_id, data_t *pattern_data, data_t *bit_const_data)
{
    if (!hash_check_key(enum_array, enum_id->strVal))
        fail_error("No enum named '%s'.", enum_id->strVal);


    int len = strlen(pattern_data->strVal) + 1; // +1 to count the terminating NULL

    xmalloc_set_handler(xmalloc_callback);
    enumeration_t *enumeration = (enumeration_t *)hash_get(enum_array, enum_id->strVal);
    pattern_t* pattern = xmalloc(sizeof(pattern_t) + sizeof(char [len]));
    pattern->bit_const = bit_const_data->bVal;
    strcpy(pattern->pattern, pattern_data->strVal);

    if (pattern->bit_const.width != enumeration->width)
    {
        fail_warning("The pattern \"%s\" width doesn't match the enum '%s' width."
            " Automatically trucated to %i bits.",
            pattern->pattern,
            enumeration->name,
            enumeration->width);
        pattern->bit_const.width = enumeration->width; //Force the bit_constant to have the correct with
    }

    //Create the list or append it
    if (enumeration->pattern_list == NULL)
        enumeration->pattern_list = list_init(0, (void*)pattern, eDATA);
    else
        list_append(enumeration->pattern_list, list_init(0, (void*)pattern, eDATA));
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