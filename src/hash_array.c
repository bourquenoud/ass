#include "hash_array.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "xmalloc.h"
#include "macro.h"

bucket_t *bucket_init(char *key, void *user_data);
void bucket_add(bucket_t *self, bucket_t *new_bucket);
bucket_t *bucket_find(bucket_t *self, char *key);
uint32_t bucket_count(bucket_t *self);
size_t bucket_push_to_array(bucket_t *self, bucket_t **serialised_data);

uint32_t hash(unsigned char *str);
static void xmalloc_callback(int err);

/********************************************************/

bucket_t *bucket_init(char *key, void *user_data)
{
    bucket_t *new_bucket;
    int len = strlen(key) + 1;
    xmalloc_set_handler(xmalloc_callback);
    new_bucket = xmalloc(sizeof(bucket_t) + len);
    new_bucket->user_data = user_data;
    new_bucket->previous = NULL; // Initalised as a floating bucket
    new_bucket->next = NULL;
    strcpy(new_bucket->key, key);
    return new_bucket;
}

void bucket_add(bucket_t *self, bucket_t *new_bucket)
{
    bucket_t *current = self;

    // Find the last bucket of the list
    while (current->next)
    {
        current = current->next;
    }

    // Add the bucket to the end
    current->next = new_bucket;
    new_bucket->previous = current;
}

/* TODO: Implement a way to remove buckets
bucket_t* bucket_remove(bucket_t* self, char* key)
{
    bucket_t* current_bucket = bucket_find();
    if
}
*/

bucket_t *bucket_find(bucket_t *self, char *key)
{
    bucket_t *current = self;

    // Find the last bucket of the list
    while (current)
    {
        if (strcmp(current->key, key) == 0)
            return current;

        current = current->next;
    }

    return NULL;
}

uint32_t bucket_count(bucket_t *self)
{
    uint32_t count = 1; // There's at least on element (self)

    bucket_t *current = self;

    // Find the last bucket of the list
    while (current->next)
    {
        count++; // And count one for each
        current = current->next;
    }

    return count;
}

// Push from the beginning of the arry
size_t bucket_push_to_array(bucket_t *self, bucket_t **serialised_data)
{
    uint32_t count = 0;
    bucket_t *current = self;

    // Goes throught all elements and push them on the array utill the end of the array
    while (current)
    {
        serialised_data[0] = current;
        count++; // And count one for each
        current = current->next;
    }

    // Return the numer of element pushed
    return count;
}

// Deallocate all buckets
void bucket_free(bucket_t *self)
{
    bucket_t *current = self;
    bucket_t *next;

    // Goes throught all elements and push them on the array utill the end of the array
    while (current)
    {
        next = current->next;
        free(current);
        current = next;
    }
}

/********************************************************/

hash_t *hash_init(int size)
{
    hash_t *new_hash_array;
    xmalloc_set_handler(xmalloc_callback);
    new_hash_array = xmalloc(sizeof(hash_t) + sizeof(bucket_t[size]));  // Allocate the struct with its flexible array member
    memset(new_hash_array, 0, sizeof(hash_t) + sizeof(bucket_t[size])); // Set everything to NULL
    new_hash_array->size = size;
    return new_hash_array;
}

void hash_add(hash_t *self, char *key, void *data)
{
    // Create a bucket for holding the data
    bucket_t *new_bucket = bucket_init(key, data);

    // Get the hash
    uint32_t hash_val = hash(key) % self->size;

    // Get the current content of hash cell
    bucket_t *current_bucket = self->buckets[hash_val];

    // Add the new bucket if it is currently empty, or append to the end if not empty
    if (current_bucket == NULL)
        self->buckets[hash_val] = new_bucket;
    else
        bucket_add(self->buckets[hash_val], new_bucket);
}

// Get the data from the hash if it exists, or return NULL
void *hash_get(hash_t *self, char *key)
{
    // Get the hash
    uint32_t hash_val = hash(key) % self->size;

    // Get the current content of hash cell
    bucket_t *current_bucket = self->buckets[hash_val];

    // Hash to an empty slot then the key doesn't exist
    if (current_bucket == NULL)
        return NULL;

    // Search the bucket list for a match, will return a NULL if nothing has been found
    return bucket_find(current_bucket, key)->user_data;
}

// Try to get the data, if it doesn't exist return false. Data contains the data if it exists
bool hash_try_get(hash_t *self, char *key, void **data)
{
    // Get the hash
    uint32_t hash_val = hash(key) % self->size;

    // Get the current content of hash cell
    bucket_t *current_bucket = self->buckets[hash_val];

    // Hash to an empty slot then the key doesn't exist
    if (current_bucket == NULL)
        return false;

    // Search the bucket list for a match, will return a NULL if nothing has been found
    bucket_t *bucket = bucket_find(current_bucket, key);
    if (bucket == NULL)
        return false;

    *data = bucket->user_data;
    return true;
}

bool hash_check_key(hash_t *self, char *key)
{
    return (hash_get(self, key) != NULL);
}

size_t hash_count(hash_t *self)
{
    uint32_t count = 0;
    for (int i = 0; i < self->size; i++)
    {
        // Add the numer of elements in one slot
        if (self->buckets[i])
            count += bucket_count(self->buckets[i]);
    }
    return count;
}

bucket_t **hash_serialise(hash_t *self)
{
    bucket_t **serialised_data;
    uint32_t n_elements = hash_count(self);

    if (n_elements <= 0)
        return NULL;

    xmalloc_set_handler(xmalloc_callback);
    serialised_data = xmalloc(sizeof(bucket_t *) * n_elements);

    uint32_t count = 0;
    int i = 0;
    while (count < n_elements)
    {
        if (self->buckets[i])
            count += bucket_push_to_array(self->buckets[i], serialised_data + count);
        i++;
    }

    return serialised_data;
}

// Dealocate all buckets and the hash table
void hash_free(hash_t *self)
{
    for (int i = 0; i < self->size; i++)
    {
        if (self->buckets[i])
            bucket_free(self->buckets[i]);
    }
    free(self);
}

/********************************************************/

// djb2 hash
uint32_t hash(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

/********************************************************/

void xmalloc_callback(int err)
{
    fputs("\033[31mError in " STR(__FILE__) " : ", stderr);
    if (0 == err)
        fputs("Cannot allocate zero length memory\033[0m\n", stderr);
    else if (1 == err)
        fputs("Malloc returned a NULL pointer\033[0m\n", stderr);
    else
        fputs("Unknown errro\033[0m\n", stderr);
}