#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include "macro.h"

typedef struct
{
    size_t element_size;
    size_t size;
    size_t count;
    uint8_t element_list[]; //Byte array
}darray_t;

/**
 * @brief Initialise a dynamic array with a specific element size
 * 
 * @param element_size The size in bytes of an element (sizeof(TYPE))
 * @return darray_t* The newly created empty dynamic array
 */
darray_t *darray_init(size_t element_size);

/**
 * @brief Add an element to the array, and grow it if necessary
 * 
 * @param array A pointer to the array to modify
 * @param data A pointer to the data
 * @return darray_t* The new array pointer if the data block has moved.
 *         Otherwise returns the original array pointer array
 */
darray_t *darray_add(darray_t *array, const void const *data);

/**
 * @brief Remove n elements from the end
 * 
 * @param array A pointer to the array to modify
 * @param count The number of elements to remove
 * @return darray_t* The new array pointer if the data block has moved.
 *         Otherwise returns the original array pointer array
 */
darray_t *darray_remove(darray_t* array, int count);

/**
 * @brief Get the data at a specific index
 * 
 * @param array A pointer to array to get the data from 
 * @param data A pointer to where the data will be stored
 * @param index The index to the element we want to get
 */
void darray_get(darray_t *array, void* data, int index);
