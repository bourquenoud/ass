#pragma once

#include <stdlib.h>

enum
{
    eNODE,
    eDATA,
    eBIT_ELEM
};

typedef struct linked_list_s linked_list_t;
struct linked_list_s
{
    int type;
    void* user_data; // FIXME: <--- THIS IS A DATA LEAK.  USER_DATA IS NEVER FREED
    int data_type;
    linked_list_t* next;
};

/**
 * @brief Create a new linked list
 * @return The new linked list
 */
linked_list_t* list_init(int type, void* data, int data_type);

/**
 * @brief Add a new node to the linked list
 * 
 * @param self The linked list to add the node to
 * @param new_element The new node to add
 */
void list_append(linked_list_t* self, linked_list_t* new_element);

/**
 * @brief Insert a new node into the linked list after the specified node position
 * 
 * @param self The linked list to insert the node into
 * @param index The index of the node to insert after
 * @param new_element The new node to insert
 */
void list_insert_after(linked_list_t* self, int index, linked_list_t* new_element);

/**
 * @brief Remove a node from the linked list at the specified index
 * 
 * @param self The linked list to remove the node from
 * @param index The index of the node to remove
 * @return linked_list_t* The starting node of the linked list (self if index is not 0)
 */
linked_list_t* list_remove_at(linked_list_t* self, int index);

/**
 * @brief Destroy the linked list
 * 
 * @param self The linked list to destroy
 */
void list_destroy(linked_list_t* self);

/**
 * @brief Get the data at the specified index
 * 
 * @param self The linked list to get the data from
 * @param index The index of the data to get
 * @return void* The data at the specified index
 */
linked_list_t* list_get_at(linked_list_t* self, int index);

/**
 * @brief Get the last element of the linked list
 * 
 * @param self The linked list to get the last element from
 * @return linked_list_t* The last element of the linked list
 */
linked_list_t* list_get_last(linked_list_t* self);

/**
 * @brief Get the size of the linked list
 * 
 * @param self The linked list to get the size of
 * @return int The size of the linked list
 */
int list_get_lenght(linked_list_t* self);


void print_list(linked_list_t* self);


void print_list_enum(linked_list_t* self);