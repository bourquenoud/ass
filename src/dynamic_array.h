#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>
#include "macro.h"

typedef struct
{
    size_t element_size;
    size_t size;
    size_t count;
    uint8_t element_list[]; // Byte array
} darray_t;

/**
 * @brief Initialise a dynamic array with a specific element size
 *
 * @param element_size The size in bytes of an element (sizeof(TYPE))
 * @return darray_t* The newly created empty dynamic array
 */
darray_t *darray_init(size_t element_size);

// Wrapped function
void _darray_add(darray_t **array, const void const *data);
/**
 * @def darray_add(array, data)
 * @brief Add an element to the array, and grow it if necessary
 *
 * @param array A pointer to the array to modify
 * @param data The data
 * @return darray_t* The new array pointer if the data block has moved.
 *         Otherwise returns the original array pointer array
 */
#define darray_add(array, data) _darray_add((array), (&(data)))

/**
 * @brief Remove n elements from the end
 *
 * @param array A pointer to the array to modify
 * @param count The number of elements to remove
 * @return darray_t* The new array pointer if the data block has moved.
 *         Otherwise returns the original array pointer array
 */
void darray_remove(darray_t **array, int count);

/**
 * @brief Remove elements at the index and shift the array if necessary
 *
 * @details Remove the specified amount of elements from the array
 * and the shift it. It removes the element at the specified index,
 * and the one following it in increasing index order. It does NOT
 * check for array overflow.
 *
 * @param array A pointer to the array to modify
 * @param count The number of element to remove
 * @param index The index of the first element to remove
 */
void darray_remove_at(darray_t **array, int count, int index);

/**
 * @brief Get the data at a specific index and store the result
 *
 * @param array A pointer to array to get the data from
 * @param data A pointer to where the data will be stored
 * @param index The index to the element we want to get
 */
void darray_get(darray_t **array, void *data, int index);

/**
 * @brief Return a pointer to the data address
 *
 * @param array A pointer to the dynmaic array
 * @param index The index in of the element
 * @return void* The address of the element
 */
void *darray_get_ptr(darray_t **array, int index);

/**
 * @brief Copy a dynamic array
 *
 * @param array_dest Pointer to the destination array
 * @param array_src Pointer to the source array
 */
void darray_copy(darray_t **array_dest, darray_t **array_src);

/**
 * @brief Resize allocated memory of the array
 * @details Reallocate the memory for the array, and remove
 * elements if the new size is smaller than the number of elements.
 *
 * @param array Pointer to the array to resize
 * @param size Size in number of elements
 */
void darray_resize(darray_t **array, int size);

/**
 * @brief Remove all elements of the array and
 * resize it to the default size
 *
 */
void darray_empty(darray_t **array);

/**
 * @brief Delete an array
 *
 * @param array Pointer to the array to delete
 */
void darray_free(darray_t **array);

/**
 * @brief Print to a dynamic array
 *
 * @param array Pointer to a dynamic array to print to
 * @param format printf style format
 * @param ... arguments
 */
void bprintf(darray_t **array, const char *format, ...);