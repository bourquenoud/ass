#pragma define

#include <stdlib.h>

#include "ast_node.h"

typedef struct linked_list_s linked_list_t;
struct linked_list_s
{
    int type;
    data_t data;
    linked_list_t* next;
};

linked_list_t* list_init(int type, data_t data);
void list_append(linked_list_t* self, linked_list_t* new_element);
void list_insert_after(linked_list_t* self, int index, linked_list_t* new_element);
linked_list_t* list_remove_at(linked_list_t* self, int index);
void list_destroy(linked_list_t* self);
linked_list_t* list_get_at(linked_list_t* self, int index);
linked_list_t* list_get_last(linked_list_t* self);
