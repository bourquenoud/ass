#pragma once

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    uint64_t val;
    uint8_t width;
} bit_const_t;

typedef union
{
    char *strVal;
    int64_t iVal;
    uint64_t uVal;
    double dVal;
    bit_const_t bVal;
} data_t;

typedef struct node_s node_t;
// Create the node structure for the AST
struct node_s
{
    int type;
    int count;
    data_t value;
    node_t *parent;
    node_t *child_1;
    node_t *child_2;
    node_t *child_3;
};

// Tree is build bottom-up
node_t *node_init(int type, data_t value, node_t *child_1, node_t *child_2, node_t *child_3);
void node_destroy(node_t *self);
node_t *node_clone(node_t *self);
void node_copy(node_t *self, node_t *node);
void node_empty(node_t *self);
void node_print(node_t *self, int depth, uint64_t drawCol);