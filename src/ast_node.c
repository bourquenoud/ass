#include "ast_node.h"

#include <string.h>
#include <stdio.h>

#include "generated/ass.tab.h"
#include "xmalloc.h"

#include "macro.h"

static void xmalloc_callback(int err);

/***********************************************************************/
/*                          AST NODES                                  */
/***********************************************************************/
// We use pseudo-objects for the tree's nodes

// Tree is build bottom-up
node_t *node_init(int type, data_t value, node_t *child_1, node_t *child_2, node_t *child_3)
{
    xmalloc_set_handler(xmalloc_callback);
    node_t *newNode = xmalloc(sizeof(node_t));
    if (newNode == NULL)
    {
        fputs("Unable to allocate memory for the AST", stderr);
        exit(1);
    }

    newNode->type = type;
    newNode->value = value;
    newNode->count = 0;
    newNode->parent = NULL;

    newNode->child_1 = child_1;
    if (child_1 != NULL)
        ++newNode->count;

    newNode->child_2 = child_2;
    if (child_2 != NULL)
        ++newNode->count;

    newNode->child_3 = child_3;
    if (child_3 != NULL)
        ++newNode->count;

    return newNode;
}

void node_destroy(node_t *self)
{
    node_empty(self);
    free(self);
}

// Copy the nodes values and return a new node
node_t *node_clone(node_t *self)
{
    xmalloc_set_handler(xmalloc_callback);
    node_t *newNode = xmalloc(sizeof(node_t));
    node_copy(self, newNode);
    return newNode;
}

// Copy the node values to a node
void node_copy(node_t *self, node_t *node)
{
    node->type = self->type;
    node->count = self->count;
    node->value = self->value;
    node->parent = self->parent;
    node->child_1 = self->child_1;
    node->child_2 = self->child_2;
    node->child_3 = self->child_3;
}

// Free all the children of a node
// NOTE: Double recursivity (empty->destroy->empty->destroy->...)
void node_empty(node_t *self)
{
    // Ask children to free their memory
    if (self->child_1 != NULL)
        node_destroy(self->child_1);
    if (self->child_2 != NULL)
        node_destroy(self->child_2);
    if (self->child_3 != NULL)
        node_destroy(self->child_3);

    // Remove its own children
    if (self->child_1 != NULL)
        free(self->child_1);
    if (self->child_2 != NULL)
        free(self->child_2);
    if (self->child_3 != NULL)
        free(self->child_3);
}

// Print the tree structure
void node_print(node_t *self, int depth, uint64_t drawCol)
{
    int depthInc = 1;

    if (self == NULL)
        return;

    if (!strcmp("statement", getTypeName(self->type)))
    {
        depthInc = 0;
    }
    else
    {

        for (int i = 0; i < depth; i++)
        {
            if (((drawCol) >> i) & 1)
                fputs("\u2502  ", stdout);
            else
                fputs("   ", stdout);
        }

        if (((drawCol) >> depth) & 1)
            fputs("\u251C", stdout);
        else
            fputs("\u2514", stdout);

        fputs("\u2500\u2500", stdout);
        printf("%s", getTypeName(self->type));

        /*
                switch (self->type)
                {
                case T_LITINTEGER:
                    printf("(\x1b[96m%i\x1b[0m)", self->value.iVal); //Int are blue
                    break;
                case T_LITDOUBLE:
                    printf("(\x1b[95m%f\x1b[0m)", self->value.dVal); //Float are violet
                    break;
                case T_ID:
                    printf("(\x1b[92m%s\x1b[0m)", self->value.strVal); //Vars/functions are green
                    break;
                case T_LITSTR:
                    printf("(\x1b[33;3m%s\x1b[0m)", self->value.strVal); //Strings are italic yellow
                default:
                    break;
                }
                */
        putc('\n', stdout);
    }

    if (self->child_1 != NULL)
        node_print(self->child_1, depth + depthInc, ((uint64_t)(self->child_2 != NULL) << depth + depthInc) | drawCol);
    if (self->child_2 != NULL)
        node_print(self->child_2, depth + depthInc, ((uint64_t)(self->child_3 != NULL) << depth + 1) | drawCol);
    if (self->child_3 != NULL)
        node_print(self->child_3, depth + depthInc, drawCol);
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