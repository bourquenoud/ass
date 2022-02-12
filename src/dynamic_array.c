#include "dynamic_array.h"

#define DEFAULT_DARRAY_SIZE 16

void _darray_add(darray_t **array, const void const *data);

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

void _darray_add(darray_t **array, const void const *data)
{
    if ((*array)->count >= (*array)->size)
    {
        (*array) = realloc((*array), sizeof(darray_t) + ((*array)->element_size) * ((*array)->size) * 2);
        (*array)->size *= 2;
    }

    memcpy((*array)->element_list + ((*array)->element_size * (*array)->count), data, (*array)->element_size);
    (*array)->count++;
}

void darray_get(darray_t **array, void *data, int index)
{
    size_t computed_index = index * (*array)->element_size;

    // Copy all bytes to the array
    for (int i = 0; i < (*array)->element_size; i++)
    {
        *((uint8_t *)data + i) = (*array)->element_list[computed_index + i];
    }
}

void *darray_get_ptr(darray_t **array, int index)
{
    size_t computed_index = index * (*array)->element_size;

    return (void *)((*array)->element_list + computed_index);
}

void darray_copy(darray_t **array_dest, darray_t **array_src)
{
    darray_resize(array_dest, (*array_src)->size);
    (*array_dest)->count = (*array_src)->count;
    memcpy(&((*array_dest)->element_list), &((*array_src)->element_list), (*array_src)->count * (*array_src)->element_size);
}

void darray_resize(darray_t **array, int size)
{
    *array = realloc(*array, sizeof(darray_t) + size * (*array)->element_size);
    (*array)->size = size;
    (*array)->count = ((*array)->count > (*array)->size) ? (*array)->size : (*array)->count;
}

void darray_remove(darray_t **array, int count)
{
    (*array)->count = ((*array)->count - count < (*array)->count) ? (*array)->count - count : 0;

    if ((*array)->count <= (*array)->size / 2 && (*array)->count >= DEFAULT_DARRAY_SIZE)
    {
        (*array) = realloc((*array), sizeof(darray_t) + ((*array)->element_size) * ((*array)->size) / 2);
        (*array)->size /= 2;
    }
}

void darray_remove_at(darray_t **array, int count, int index)
{
    // Shift the bytes in the array, overriding the element to remove
    int offset = (*array)->element_size * count;

    void *dest_ptr = (*array)->element_list + ((*array)->element_size * index);
    void *src_ptr = dest_ptr + offset;
    void *end_ptr = (*array)->element_size * (*array)->count + (*array)->element_list;

    int size = (int)(end_ptr - src_ptr);

    memmove(dest_ptr, src_ptr, size);

    // Last element is now duplicated, remove it
    darray_remove(array, count);
}