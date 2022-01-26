#include "enumerations.h"

hash_t* enum_array;

int command_enum(data_t *id, data_t *value)
{
    //Check if the enum already exists
    if (hash_check_key(enum_array, id->strVal))
    {
        printf("'%s' redeclared. You can not declare a enum twice.\n", id->strVal);
        return -1;
    }

    hash_add(enum_array, id->strVal, (void*)value);
    return 0; //Success
}