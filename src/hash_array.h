#pragma once

#include <stdbool.h>
#include <stdlib.h>

typedef struct bucket_s bucket_t;

struct bucket_s
{
    void *user_data;
    bucket_t *previous;
    bucket_t *next;
    char key[]; // Flexible array member
};

typedef struct
{
    int size;
    bucket_t *buckets[]; // Flexible array member
} hash_t;

/**
 * @brief Initialise a hash array
 *
 * @param size The size of the hash array
 * @return hash_t* Return a pointer to the newly created hash array
 */
hash_t *hash_init(int size);

/**
 * @brief Try to find an element in the hash array
 *
 * @param self The hash array
 * @param key The key of the element to find
 * @param data The data of the element to find
 * @return bucket_t* Return a pointer to the bucket if found, NULL otherwise
 */
bool hash_try_get(hash_t *self, char *key, void **data);

/**
 * @brief Add an element to the hash array
 *
 * @param self The hash array
 * @param key The key of the element to add
 * @param data The data of the element to add
 */
void hash_add(hash_t *self, char *key, void *data);

/**
 * @brief Get an element in the hash array
 *
 * @param self The hash array
 * @param key The key of the element to get
 * @return void* Return a pointer to the data of the element, NULL if not found
 */
void *hash_get(hash_t *self, char *key);

/**
 * @brief Check if an element is in the hash array
 *
 * @param self The hash array
 * @param key The key of the element to check
 * @return true If the element is in the hash array
 * @return false If the element is not in the hash array
 */
bool hash_check_key(hash_t *self, char *key);

/**
 * @brief Convert the hash array to an array
 *
 * @param self The hash array
 * @return bucket_t** Return a pointer to the array of buckets, skipping empty buckets
 */
bucket_t **hash_serialise(hash_t *self);

/**
 * @brief Get the number of elements in the hash array
 *
 * @param self The hash array
 * @return size_t The number of elements in the hash array
 */
size_t hash_count(hash_t *self);

/**
 * @brief Free the memory used by the hash array and all its elements
 *
 * @param self The hash array
 */
void hash_free(hash_t *self);