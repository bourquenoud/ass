#include "constants.h"

hash_t* const_array;

int command_const(data_t *id, data_t *value)
{
    //Check if the constant already exists
    if (hash_check_key(const_array, id->strVal))
    {
        printf("'%s' redeclared. You can not declare a constant twice.\n", id->strVal);
        return -1;
    }

    bit_const_t* data = &(value->bVal);

    hash_add(const_array, id->strVal, (void*)data);
    return 0; //Success
}