#include "bitarray.h"

void bitarray_set(bitarray_t *bitarray, int index, bool val)
{
    int uint32_index = index / 32;
    uint32_t mask = 1 << (index % 32);
    if (val)
        bitarray->data[uint32_index] |= mask;
    else
        bitarray->data[uint32_index] &= ~mask;
}

void bitarray_set_all(bitarray_t *bitarray, bool val)
{
    uint32_t memcontent = (val) ? 0xFFFFFFFF : 0x00000000;
    memset(bitarray->data, memcontent, sizeof(bitarray->data));
}

bitarray_t bitarray_union(bitarray_t *bitarray_a, bitarray_t *bitarray_b)
{
    bitarray_t new_bitarray;

    for (size_t i = 0; i < BITARRAY_WIDTH / 32; i++)
    {
        new_bitarray.data[i] = bitarray_a->data[i] | bitarray_b->data[i];
    }
    return new_bitarray;
}

bitarray_t bitarray_intersection(bitarray_t *bitarray_a, bitarray_t *bitarray_b)
{
    bitarray_t new_bitarray;

    for (size_t i = 0; i < BITARRAY_WIDTH / 32; i++)
    {
        new_bitarray.data[i] = bitarray_a->data[i] & bitarray_b->data[i];
    }
    return new_bitarray;
}

bool bitarray_compare(bitarray_t *bitarray_a, bitarray_t *bitarray_b)
{
    for (size_t i = 0; i < BITARRAY_WIDTH / 32; i++)
    {
        if (bitarray_a->data[i] != bitarray_b->data[i])
            return false;
    }
    return true;
}

bool bitarray_get(bitarray_t *bitarray, int index)
{
    int uint32_index = index / 32;
    uint32_t mask = 1 << (index % 32);
    return (bitarray->data[uint32_index] & mask) ? true : false;
}