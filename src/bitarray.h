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

void bitarray_set(bitarray_t *bitarray, int index, bool val);
void bitarray_set_all(bitarray_t *bitarray, bool val);
bitarray_t bitarray_union(bitarray_t *bitarray_a, bitarray_t *bitarray_b);
bitarray_t bitarray_intersection(bitarray_t *bitarray_a, bitarray_t *bitarray_b);
bool bitarray_compare(bitarray_t *bitarray_a, bitarray_t *bitarray_b);
bool bitarray_get(bitarray_t *bitarray, int index);
