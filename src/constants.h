#pragma once

#include <stdint.h>

#include "linked_list.h"
#include "ast_node.h"
#include "hash_array.h"

typedef struct pattern_s
{
    bit_const_t bit_const;
    char pattern[]; //Flexible array
} pattern_t;

typedef struct
{
    linked_list_t* pattern_list;
    int width;
    char name[]; //Flexible array
} enumeration_t;

extern hash_t *bit_const_array;
extern hash_t *int_const_array;
extern hash_t *str_const_array;
extern hash_t *enum_array;
extern hash_t *format_array;

// id expects a string, value expects a uint64_t
int command_bit_const(data_t *id, data_t *value);
int command_int_const(data_t *id, data_t *value);
int command_str_const(data_t *id, data_t *value);
int command_enum(data_t *id, data_t *value);
int command_format(data_t *id, linked_list_t *value);
int command_order(data_t *id, linked_list_t *order_args);
int command_pattern(data_t *enum_id, data_t *pattern, data_t *bit_const);