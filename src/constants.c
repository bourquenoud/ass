#include "constants.h"

hash_t* bit_const_array;
hash_t* int_const_array;

int command_bit_const(data_t *id, data_t *value)
{
    //Check if the constant already exists
    if (hash_check_key(bit_const_array, id->strVal))
    {
        printf("'%s' redeclared. You can not declare a constant twice.\n", id->strVal);
        return -1;
    }

    bit_const_t* data = &(value->bVal);

    hash_add(bit_const_array, id->strVal, (void*)data);
    return 0; //Success
}


int command_const(data_t *id, data_t *value)
{
    //Check if the constant already exists
    if (hash_check_key(int_const_array, id->strVal))
    {
        printf("'%s' redeclared. You can not declare a constant twice.\n", id->strVal);
        return -1;
    }

    hash_add(int_const_array, id->strVal, (void*)value);
    return 0; //Success
}