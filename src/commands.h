#pragma once

#include <stdint.h>

#include "parameters.h"
#include "dynamic_array.h"
#include "linked_list.h"
#include "ast_node.h"
#include "hash_array.h"

typedef struct pattern_s
{
    bit_const_t bit_const;
    char pattern[]; // Flexible array
} pattern_t;

typedef struct
{
    linked_list_t *pattern_list;
    int width;
    char name[]; // Flexible array
} enumeration_t;

typedef struct
{
    char *text_pattern;
    linked_list_t *bit_pattern;
} opcode_t;

extern hash_t *bit_const_array;
extern hash_t *int_const_array;
extern hash_t *str_const_array;
extern hash_t *enum_array;
extern hash_t *format_array;
extern darray_t *opcode_array;

void command_init();

int command_bit_const(data_t *id, data_t *value);
int command_int_const(data_t *id, data_t *value);
int command_str_const(data_t *id, data_t *value);
int command_enum(data_t *id, data_t *value);
int command_format(data_t *id, linked_list_t *value);
int command_order(data_t *id, linked_list_t *order_args);
int command_opcode(data_t *id, data_t *pattern, data_t *opcode_id, bool is_constant);
int command_pattern(data_t *enum_id, data_t *pattern, data_t *bit_const);