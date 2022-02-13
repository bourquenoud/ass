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

linked_list_t* list_init(int type, void* data, int data_type);
void list_append(linked_list_t* self, linked_list_t* new_element);
void list_insert_after(linked_list_t* self, int index, linked_list_t* new_element);
linked_list_t* list_remove_at(linked_list_t* self, int index);
void list_destroy(linked_list_t* self);
linked_list_t* list_get_at(linked_list_t* self, int index);
linked_list_t* list_get_last(linked_list_t* self);
int list_get_lenght(linked_list_t* self);

void print_list(linked_list_t* self);
void print_list_enum(linked_list_t* self);