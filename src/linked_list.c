#include "linked_list.h"

#include "ast_node.h"
#include "xmalloc.h"
#include "macro.h"
#include "generated/ass.tab.h"
#include "commands.h"

static void xmalloc_callback(int err);

linked_list_t *list_init(int type, void *user_data, int data_type)
{
    linked_list_t *new_list;
    xmalloc_set_handler(xmalloc_callback);
    new_list = xmalloc(sizeof(linked_list_t));
    new_list->type = type;
    new_list->user_data = user_data;
    new_list->data_type = data_type;
    new_list->next = NULL;
    return new_list;
}

void list_append(linked_list_t *self, linked_list_t *new_element)
{
    linked_list_t *last = list_get_last(self);
    last->next = new_element;
}

void list_insert_after(linked_list_t *self, int index, linked_list_t *new_element)
{
    linked_list_t *before = list_get_at(self, index);
    linked_list_t *after = before->next;

    before->next = new_element;
    new_element->next = after;
}

linked_list_t *list_remove_at(linked_list_t *self, int index)
{
    if (0 < index)
    {
        linked_list_t *before = list_get_at(self, index - 1);
        linked_list_t *to_remove = (before) ? before->next : NULL; // Return NULL to_remove haven't been found
        linked_list_t *after = (to_remove) ? after->next : NULL;   // Return NULL if there's no element after the one to remove

        list_destroy(to_remove); // Checked for NULL in list_destroy
        if (before)              // Check for NULL
            before->next = after;

        return self; // Starting point hasn't changed
    }
    else if (0 == index) // Remove the first element in the list
    {
        linked_list_t *new_start = self;
        new_start = self->next;
        list_destroy(self);
        return new_start;
    }
    else // Search from end
    {
        printf("Tried to reverse index in a linked list. This feature is not implemented!\n");
        abort();
    }
}

void list_destroy(linked_list_t *self)
{
    if (self)
        free(self);
}

linked_list_t *list_get_at(linked_list_t *self, int index)
{
    if (0 < index) // Search from start
    {
        linked_list_t *current = self->next; // Skip the first
        index--;
        while (current && 0 < index--) // Stop when current is NULL or index has been reached
        {
            current = current->next;
        }
        return current; // Will return a NULL if the index hasn't been found
    }
    else if (0 == index) // Zero, return self
    {
        return self;
    }
    else // Search from end
    {
        printf("Tried to reverse index in a linked list. This feature is not implemented!\n");
        abort();
    }
}

linked_list_t *list_get_last(linked_list_t *self)
{
    linked_list_t *current = self;
    while (current->next)
    {
        current = current->next;
    }
    return current;
}

int list_get_lenght(linked_list_t *self)
{
    if (self == NULL)
        return 0;

    int i = 0;
    linked_list_t *current = self;
    while (current->next)
    {
        current = current->next;
        i++;
    }
    return i + 1;
}

void print_list(linked_list_t *self)
{
    linked_list_t *current = self;
    while (current)
    {
        if (current->data_type == eNODE)
        {
            printf("Type %s\n", getTypeName(current->type));
            node_print((node_t *)current->user_data, 0, 0);
        }
        else
        {
            printf("Type %s with data\n", getTypeName(current->type));
        }

        current = current->next;
    }
}

void print_list_enum(linked_list_t *self)
{
    linked_list_t *current = self;
    while (current)
    {
        if (current->data_type == eNODE)
        {
            printf("Type %s\n", getTypeName(current->type));
            node_print((node_t *)current->user_data, 0, 0);
        }
        else
        {
            printf("  pattern \"%s\" of value %#lx\n", ((pattern_t *)(current->user_data))->pattern, ((pattern_t *)(current->user_data))->bit_const.val);
        }

        current = current->next;
    }
}

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