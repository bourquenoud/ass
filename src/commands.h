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
    int token_id;
    linked_list_t *pattern_list;
    int width;
    char name[]; // Flexible array
} enumeration_t;

typedef struct
{
    int token_id;
    char *text_pattern;
    linked_list_t *bit_pattern;
} opcode_t;

typedef struct
{
    char* name;
    char* description;
    char* code;
} custom_output_t;

extern hash_t *bit_const_array;
extern hash_t *int_const_array;
extern hash_t *str_const_array;
extern hash_t *enum_array;
extern hash_t *format_array;
extern darray_t *opcode_array;
extern darray_t *custom_output_array;

extern char* override_code[1];
extern char* code;

void command_init();

int command_bit_const(data_t *id, data_t *value);
int command_int_const(data_t *id, data_t *value);
int command_str_const(data_t *id, data_t *value);
int command_enum(data_t *id, data_t *value);
int command_format(data_t *id, linked_list_t *value);
int command_order(data_t *id, linked_list_t *order_args);
int command_opcode(data_t *id, data_t *pattern, data_t *opcode_id, bool is_constant);
int command_pattern(data_t *enum_id, data_t *pattern, data_t *bit_const);
int command_code(data_t* in_code);
int command_output(data_t *name, data_t *description, data_t *in_code);
int command_override(data_t* target_name, data_t* in_code);