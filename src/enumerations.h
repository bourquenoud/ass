#pragma once

#include "linked_list.h"
#include "ast_node.h"
#include "hash_array.h"

extern hash_t* enum_array;

// id expects a string, value expects a uint64_t
int command_enum(data_t *id, data_t *value);
