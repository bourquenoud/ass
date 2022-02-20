#include "bitpattern.h"

#include "failure.h"

static void xmalloc_callback(int err);

bit_elem_t *bit_elem_init(BPTYPE_t type, size_t width, void *data)
{
    bit_elem_t *new_bit_elem = NULL;
    int data_len = 0;
    bit_const_t *bc_val = NULL;
    enumeration_t *enum_val = NULL;

    xmalloc_set_handler(xmalloc_callback);

    switch (type)
    {
    // Substitutions
    case eBP_LABEL_REL:
    case eBP_LABEL_ABS:
    case eBP_IMMEDIATE:
        // Expects no data
        new_bit_elem = xmalloc(sizeof(bit_elem_t) + 0);
        new_bit_elem->index_mnemonic = 0;
        new_bit_elem->data_len = 0;
        new_bit_elem->width = width;
        break;

    case eBP_ID:
        // Expects no data
        new_bit_elem = xmalloc(sizeof(bit_elem_t) + 0);
        new_bit_elem->index_mnemonic = -1; // Non variable
        new_bit_elem->data_len = 0;
        new_bit_elem->width = width;
        break;

    case eBP_ENUM:

        // Resolve the name
        enum_val = ((enumeration_t *)hash_get(enum_array, (char *)data));

        // Expects the name of the enum
        data_len = sizeof(enumeration_t) + strlen(enum_val->name) + 1;
        if (!enum_val)
        {
            fail_error("'%s' has never been declared", (char *)data);
            break;
        }
        new_bit_elem = xmalloc(sizeof(bit_elem_t) + data_len);
        new_bit_elem->index_mnemonic = 0;
        new_bit_elem->data_len = data_len;
        new_bit_elem->width = enum_val->width;
        memcpy(new_bit_elem->data, enum_val, data_len);
        break;

    case eBP_BIT_CONST:
        // Expects the name of the bit constant
        data_len = sizeof(bit_const_t);

        // Resolve the name
        bc_val = ((bit_const_t *)hash_get(bit_const_array, (char *)data));
        if (!bc_val)
        {
            fail_error("'%s' has never been declared", (char *)data);
            break;
        }
        new_bit_elem = xmalloc(sizeof(bit_elem_t) + data_len);
        new_bit_elem->index_mnemonic = -1; // Non variable
        new_bit_elem->data_len = data_len;
        new_bit_elem->width = bc_val->width;
        memcpy(new_bit_elem->data, bc_val, data_len);
        break;

    case eBP_BIT_LIT:
        // Expects a bit literal
        data_len = sizeof(bit_const_t);

        // Resolve the name
        bc_val = (bit_const_t *)data;
        new_bit_elem = xmalloc(sizeof(bit_elem_t) + data_len);
        new_bit_elem->index_mnemonic = -1; // Non variable
        new_bit_elem->data_len = data_len;
        new_bit_elem->width = bc_val->width;
        memcpy(new_bit_elem->data, bc_val, data_len);
        break;

    case eBP_ELLIPSIS:
        // Expects no data
        new_bit_elem = xmalloc(sizeof(bit_elem_t) + 0);
        new_bit_elem->index_mnemonic = -1; // Non variable
        new_bit_elem->data_len = 0;
        new_bit_elem->width = 0;
        break;

    default:
        return NULL;
    }
    new_bit_elem->type = type;
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