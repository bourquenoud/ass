#pragma once

#include <stdbool.h>
#include <stdlib.h>

typedef struct bucket_s bucket_t;

struct bucket_s
{
    void* user_data;
    bucket_t* previous;
    bucket_t* next;
    char key[]; //Flexible array member
};

typedef struct
{
    int size;
    bucket_t* buckets[]; //Flexible array member
} hash_t;

hash_t* hash_init(int size);
void hash_add(hash_t* self, char* key, void* data);
void* hash_get(hash_t* self, char* key);
bool hash_check_key(hash_t* self, char* key);
bucket_t** hash_serialise(hash_t* self);
size_t hash_count(hash_t* self);