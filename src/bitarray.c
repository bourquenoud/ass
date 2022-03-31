#include "bitarray.h"

// Set the bit at index to val
void bitarray_set(bitarray_t *bitarray, int index, bool val)
{
    int uint32_index = index / 32;
    uint32_t mask = 1 << (index % 32);
    if (val)
        bitarray->data[uint32_index] |= mask;
    else
        bitarray->data[uint32_index] &= ~mask;
}

// Set all bits to val
void bitarray_set_all(bitarray_t *bitarray, bool val)
{
    uint32_t memcontent = (val) ? 0xFFFFFFFF : 0x00000000;
    memset(bitarray->data, memcontent, sizeof(bitarray->data));
}

// Return the union of bitarray_a and bitarray_b
bitarray_t bitarray_union(bitarray_t *bitarray_a, bitarray_t *bitarray_b)
{
    bitarray_t new_bitarray;

    for (size_t i = 0; i < BITARRAY_WIDTH / 32; i++)
    {
        new_bitarray.data[i] = bitarray_a->data[i] | bitarray_b->data[i];
    }
    return new_bitarray;
}

// Return the intersection of bitarray_a and bitarray_b
bitarray_t bitarray_intersection(bitarray_t *bitarray_a, bitarray_t *bitarray_b)
{
    bitarray_t new_bitarray;

    for (size_t i = 0; i < BITARRAY_WIDTH / 32; i++)
    {
        new_bitarray.data[i] = bitarray_a->data[i] & bitarray_b->data[i];
    }
    return new_bitarray;
}

// Return true if bitarray_a and bitarray_b are equal
bool bitarray_compare(bitarray_t *bitarray_a, bitarray_t *bitarray_b)
{
    for (size_t i = 0; i < BITARRAY_WIDTH / 32; i++)
    {
        if (bitarray_a->data[i] != bitarray_b->data[i])
            return false;
    }
    return true;
}

// Return the value of the bit at index
bool bitarray_get(bitarray_t *bitarray, int index)
{
    int uint32_index = index / 32;
    uint32_t mask = 1 << (index % 32);
    return (bitarray->data[uint32_index] & mask) ? true : false;
}