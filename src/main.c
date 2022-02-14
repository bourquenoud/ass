#include <stdlib.h>

#include "generated/ass.tab.h"
#include "commands.h"
#include "hash_array.h"
#include "parameters.h"
#include "failure.h"
#include "bitpattern.h"
#include "dynamic_array.h"

#define TABLE_SIZE 1024

int main(int argc, char **argv)
{
    enum_array = hash_init(TABLE_SIZE);
    bit_const_array = hash_init(TABLE_SIZE);
    int_const_array = hash_init(TABLE_SIZE);
    str_const_array = hash_init(TABLE_SIZE);
    format_array = hash_init(TABLE_SIZE);
    opcode_array = darray_init(sizeof(opcode_t));
    build_ast(argc, argv);

    size_t count = hash_count(enum_array);
    bucket_t **csnt = hash_serialise(enum_array);

    puts("****PARAM****");

    printf("opcode_width : %i\n", parameters.opcode_width);
    printf("memory_width : %i\n", parameters.memory_width);
    printf("alignment : %i\n", parameters.alignment);
    printf("address_width : %i\n", parameters.address_width);
    printf("address_start : %i\n", parameters.address_start);
    printf("address_stop : %i\n", parameters.address_stop);
    printf("endianness : %s\n", (parameters.endianness == 0) ? "UNDEF" : ((parameters.endianness == 1) ? "LITTLE" : "BIG"));
    printf("args_separator : %c\n", parameters.args_separator);
    printf("*label_pattern : %s\n", parameters.label_pattern);

    puts("****ENUM****");
    for (int i = 0; i < count; i++)
    {
        printf("%s = %i\n", csnt[i]->key, ((enumeration_t *)(csnt[i]->user_data))->width);
        print_list_enum(((enumeration_t *)(csnt[i]->user_data))->pattern_list);
    }

    count = hash_count(bit_const_array);
    csnt = hash_serialise(bit_const_array);

    puts("****BIT CONSTANTS****");
    for (int i = 0; i < count; i++)
    {
        printf("%s = %#x:%i\n", csnt[i]->key, ((data_t *)(csnt[i]->user_data))->bVal.val, ((data_t *)(csnt[i]->user_data))->bVal.width);
    }

    count = hash_count(int_const_array);
    csnt = hash_serialise(int_const_array);

    puts("****CONSTANTS****");
    for (int i = 0; i < count; i++)
    {
        printf("%s = %i\n", csnt[i]->key, ((data_t *)(csnt[i]->user_data))->iVal);
    }

    count = hash_count(format_array);
    csnt = hash_serialise(format_array);

    puts("****FORMATS****");
    for (int i = 0; i < count; i++)
    {
        printf("%s:\n", csnt[i]->key);
        linked_list_t *current = (csnt[i]->user_data);
        while (current != NULL)
        {
            printf("\t");
            bit_elem_t *bit_elem = (bit_elem_t *)(current->user_data);
            switch (bit_elem->type)
            {
            case eBP_ID:
                printf("[%i:%i]subst : ID(%i)", bit_elem->index_opcode, bit_elem->index_mnemonic, bit_elem->width);
                break;
            case eBP_IMMEDIATE:
                printf("[%i:%i]subst : IMMEDIATE(%i)", bit_elem->index_opcode, bit_elem->index_mnemonic, bit_elem->width);
                break;
            case eBP_LABEL_ABS:
                printf("[%i:%i]subst : LABEL_ABS(%i)", bit_elem->index_opcode, bit_elem->index_mnemonic, bit_elem->width);
                break;
            case eBP_LABEL_REL:
                printf("[%i:%i]subst : LABEL_REL(%i)", bit_elem->index_opcode, bit_elem->index_mnemonic, bit_elem->width);
                break;
            case eBP_ENUM:
                printf("[%i:%i]%s", bit_elem->index_opcode, bit_elem->index_mnemonic, ((enumeration_t *)(bit_elem->data))->name);
                break;
            case eBP_BIT_CONST:
                printf("[%i:%i]%#x:%i", bit_elem->index_opcode, bit_elem->index_mnemonic, ((bit_const_t *)(bit_elem->data))->val, ((bit_const_t *)(bit_elem->data))->width);
                break;
            case eBP_BIT_LIT:
                printf("[%i:%i]%#x:%i", bit_elem->index_opcode, bit_elem->index_mnemonic, ((bit_const_t *)(bit_elem->data))->val, ((bit_const_t *)(bit_elem->data))->width);
                break;
            case eBP_ELLIPSIS:
                printf("[%i:%i]ellipsis", bit_elem->index_opcode, bit_elem->index_mnemonic);
                break;
            default:
                printf("UNDEF");
                break;
            }
            printf("\n");
            current = current->next;
        }
    }

    puts("****OPCODES****");
    count = opcode_array->count;
    opcode_t *opcodes = darray_get_ptr(&opcode_array, 0);

    for (size_t i = 0; i < count; i++)
    {
        printf("%s:\n", opcodes[i].text_pattern);
        linked_list_t *current = (opcodes[i].bit_pattern);
        while (current != NULL)
        {
            printf("\t");
            bit_elem_t *bit_elem = (bit_elem_t *)(current->user_data);
            switch (bit_elem->type)
            {
            case eBP_ID:
                printf("[%i:%i]subst : ID(%i)", bit_elem->index_opcode, bit_elem->index_mnemonic, bit_elem->width);
                break;
            case eBP_IMMEDIATE:
                printf("[%i:%i]subst : IMMEDIATE(%i)", bit_elem->index_opcode, bit_elem->index_mnemonic, bit_elem->width);
                break;
            case eBP_LABEL_ABS:
                printf("[%i:%i]subst : LABEL_ABS(%i)", bit_elem->index_opcode, bit_elem->index_mnemonic, bit_elem->width);
                break;
            case eBP_LABEL_REL:
                printf("[%i:%i]subst : LABEL_REL(%i)", bit_elem->index_opcode, bit_elem->index_mnemonic, bit_elem->width);
                break;
            case eBP_ENUM:
                printf("[%i:%i]%s", bit_elem->index_opcode, bit_elem->index_mnemonic, ((enumeration_t *)(bit_elem->data))->name);
                break;
            case eBP_BIT_CONST:
                printf("[%i:%i]%#x:%i", bit_elem->index_opcode, bit_elem->index_mnemonic, ((bit_const_t *)(bit_elem->data))->val, ((bit_const_t *)(bit_elem->data))->width);
                break;
            case eBP_BIT_LIT:
                printf("[%i:%i]%#x:%i", bit_elem->index_opcode, bit_elem->index_mnemonic, ((bit_const_t *)(bit_elem->data))->val, ((bit_const_t *)(bit_elem->data))->width);
                break;
            case eBP_ELLIPSIS:
                printf("[%i:%i]0:%i", bit_elem->index_opcode, bit_elem->index_mnemonic, bit_elem->width);
                break;
            default:
                printf("UNDEF");
                break;
            }
            printf("\n");
            current = current->next;
        }
    }

    // Report parsing result
    int info_count = fail_get_info_count();
    if (info_count > 0)
        printf("\033[34m>>%i INFO%s<<\033[0m\n", info_count, (info_count == 1) ? "" : "S");

    int warning_count = fail_get_warning_count();
    if (warning_count > 0)
        printf("\033[33m>>%i WARNING%s<<\033[0m\n", warning_count, (warning_count == 1) ? "" : "S");

    int error_count = fail_get_error_count();
    if (error_count > 0)
    {
        printf("\033[31m>>%i ERROR%s<<\033[0m\n", error_count, (error_count == 1) ? "" : "S");
        puts("Parsing failed, exiting.");
        exit(EXIT_FAILURE);
    }
}
