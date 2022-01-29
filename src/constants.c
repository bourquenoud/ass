#include "constants.h"

#include "failure.h"

hash_t* bit_const_array;
hash_t* int_const_array;
hash_t* str_const_array;

void check_any(char* key)
{
    if ( hash_check_key(bit_const_array, key ) |
        hash_check_key(int_const_array, key ) |
        hash_check_key(str_const_array, key ))
        {
            fail_error("'%s' redeclared. You can not declare a constant twice.", key);
        }
}

int command_bit_const(data_t *id, data_t *value)
{
    //Check if the constant already exists
    check_any(id->strVal);

    bit_const_t* data = &(value->bVal);

    hash_add(bit_const_array, id->strVal, (void*)data);
    return 0; //Success
}


int command_int_const(data_t *id, data_t *value)
{
    //Check if the constant already exists
    check_any(id->strVal);

    hash_add(int_const_array, id->strVal, (void*)value);
    return 0; //Success
}

int command_str_const(data_t *id, data_t *value)
{
    //Check if the constant already exists
    check_any(id->strVal);

    hash_add(str_const_array, id->strVal, (void*)value);
    return 0; //Success
}