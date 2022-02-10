#include "dynamic_array.h"

#define DEFAULT_DARRAY_SIZE 16

darray_t *_darray_add(darray_t *array, const void const *data);

darray_t *darray_init(size_t element_size)
{
    if (element_size == 0)
    {
        fputs(STR(__FILE__) ":" STR(__LINE__) "  Trying to allocate dynamic array with an element size of 0\n", stderr);
        abort();
    }

    darray_t *new_darray;
    new_darray = malloc(sizeof(darray_t) + element_size * DEFAULT_DARRAY_SIZE);
    new_darray->element_size = element_size;
    new_darray->count = 0;
    new_darray->size = DEFAULT_DARRAY_SIZE;

    return new_darray;
}

darray_t *_darray_add(darray_t *array, const void const *data)
{
    darray_t *new_array = array;
    if (new_array->count >= new_array->size)
    {
        new_array = realloc(new_array, sizeof(darray_t) + (new_array->element_size) * (new_array->size) * 2);
        new_array->size *= 2;
    }

    //Copy all bytes to the array
    for (int i = 0; i < new_array->element_size; i++)
    {
        new_array->element_list[new_array->element_size * new_array->count + i] = *((uint8_t *)data + i);
    }
    new_array->count++;
    return new_array;
}

void darray_get(darray_t *array, void *data, int index)
{
    size_t computed_index = index * array->element_size;

    //Copy all bytes to the array
    for (int i = 0; i < array->element_size; i++)
    {
        *((uint8_t *)data + i) = array->element_list[computed_index + i];
    }
}

darray_t *darray_remove(darray_t *array, int count)
{
    darray_t *new_array = array;
    new_array->count = (new_array->count - count < new_array->count) ? new_array->count - count : 0;

    if (new_array->count <= new_array->size / 2 && new_array->count >= DEFAULT_DARRAY_SIZE)
    {
        new_array = realloc(new_array, sizeof(darray_t) + (new_array->element_size) * (new_array->size) / 2);
        new_array->size /= 2;
    }

    return new_array;
}