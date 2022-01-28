#pragma once

#include "linked_list.h"
#include "ast_node.h"
#include "hash_array.h"

extern hash_t* bit_const_array;
extern hash_t* int_const_array;
extern hash_t* str_const_array;

// id expects a string, value expects a uint64_t
int command_bit_const(data_t *id, data_t *value);
int command_int_const(data_t *id, data_t *value);
int command_str_const(data_t *id, data_t *value);