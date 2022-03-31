#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define BITARRAY_WIDTH 1024

typedef struct
{
    uint32_t data[BITARRAY_WIDTH / sizeof(uint32_t)];
} bitarray_t;

/**
 * @brief Set the bit at index to val
 * @param bitarray The bitarray to set
 * @param index The index of the bit to set
 * @param val The value to set the bit to
 */
void bitarray_set(bitarray_t *bitarray, int index, bool val);

/**
 * @brief Set all bits to val
 * @param bitarray The bitarray to set
 * @param val The value to set the bits to
 */
void bitarray_set_all(bitarray_t *bitarray, bool val);

/**
 * @brief Return the union of bitarray_a and bitarray_b
 * @param bitarray_a The first bitarray
 * @param bitarray_b The second bitarray
 * @return The union of bitarray_a and bitarray_b
 */
bitarray_t bitarray_union(bitarray_t *bitarray_a, bitarray_t *bitarray_b);

/**
 * @brief Return the intersection of bitarray_a and bitarray_b
 * @param bitarray_a The first bitarray
 * @param bitarray_b The second bitarray
 * @return The intersection of bitarray_a and bitarray_b
 */
bitarray_t bitarray_intersection(bitarray_t *bitarray_a, bitarray_t *bitarray_b);

/**
 * @brief Return true if bitarray_a and bitarray_b are equal
 * @param bitarray_a The first bitarray
 * @param bitarray_b The second bitarray
 * @return true if bitarray_a and bitarray_b are equal
 */
bool bitarray_compare(bitarray_t *bitarray_a, bitarray_t *bitarray_b);

/**
 * @brief Return the value of the bit at index
 * @param bitarray The bitarray to get the bit from
 * @param index The index of the bit to get
 * @return The value of the bit at index
 */
bool bitarray_get(bitarray_t *bitarray, int index);
