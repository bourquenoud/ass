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

typedef struct
{
    char* name;
    char* content;
} macro_t;

extern hash_t *bit_const_array;
extern hash_t *int_const_array;
extern hash_t *str_const_array;
extern hash_t *enum_array;
extern hash_t *format_array;
extern hash_t *macro_array;

extern darray_t *opcode_array;
extern darray_t *custom_output_array;

extern char* override_code[1];
extern char* code;

/**
 * @brief Initialise all arrays
 */
void command_init();

/**
 * @brief Process the command "%const" for bit constants
 * 
 * @param id The id of the constant
 * @param value The value of the constant
 * @return int Return 0 if the constant is added, 1 if the constant is already defined
 */
int command_bit_const(data_t *id, data_t *value);

/**
 * @brief Process the command "%const" for integer constants
 * 
 * @param id The id of the constant
 * @param value The value of the constant
 * @return int Return 0 if the constant is added, 1 if the constant is already defined
 */
int command_int_const(data_t *id, data_t *value);

/**
 * @brief Process the command "%const" for string constants
 * 
 * @param id The id of the constant
 * @param value The value of the constant
 * @return int Return 0 if the constant is added, 1 if the constant is already defined
 */
int command_str_const(data_t *id, data_t *value);

/**
 * @brief Process the command "%enum" for enumerations
 * 
 * @param id The id of the enumeration
 * @param value The value of the enumeration
 * @return int Return 0 if the enumeration is added, 1 if the enumeration is already defined
 */
int command_enum(data_t *id, data_t *value);

/**
 * @brief Process the command "%format"
 * 
 * @param id The id of the format
 * @param value The value of the format
 * @return int Return 0 if the format is added, 1 if the format is already defined
 */
int command_format(data_t *id, linked_list_t *value);

/**
 * @brief Process the command "%order"
 * 
 * @param id The id of the format to reorder
 * @param order_args A list of arguments to reorder the format
 * @return int Return 0 if the format is added, 1 if the format is already defined
 */
int command_order(data_t *id, linked_list_t *order_args);

/**
 * @brief Process the command "%opcode"
 * 
 * @param id The id of the format
 * @param pattern The pattern of the opcode, as a string
 * @param opcode_id The id of the opcode
 * @param is_constant If the pattern is passed as a constant
 * @return int Return 0 if the opcode is added, 1 if the opcode is already defined
 */
int command_opcode(data_t *id, data_t *pattern, data_t *opcode_id, bool is_constant);

/**
 * @brief Process the command "%pattern"
 * 
 * @param enum_id The id of the enumeration
 * @param pattern The pattern of the enumeration, as a string
 * @param bit_const The bit constant of the enumeration
 * @return int Return 0 if the pattern is added, 1 if the pattern is already defined
 */
int command_pattern(data_t *enum_id, data_t *pattern, data_t *bit_const);

/**
 * @brief Process the command "%code"
 * 
 * @param in_code The code to add
 * @return int Always return 0
 */
int command_code(data_t* in_code);

/**
 * @brief Process the command "%output"
 * 
 * @param name The name of the output type, used as the key in the CLI
 * @param description A short description of the output type
 * @param in_code A C code to be executed when the output type is selected
 * @return int Return 0 if the output is added, 1 if the output is already defined
 */
int command_output(data_t *name, data_t *description, data_t *in_code);

/**
 * @brief Process the command "%macro"
 * 
 * @param name The name of the macro
 * @param content The content of the macro
 * @return int Return 0 if the macro is added, 1 if the macro is already defined
 */
int command_macro(data_t *name, data_t *content);

/**
 * @brief Process the command "%override"
 * 
 * @param target_name The name of the code function to override
 * @param in_code A C code to be executed when the overriden function is called
 * @return int Return 0 if the override is added, 1 if the override is already defined
 */
int command_override(data_t* target_name, data_t* in_code);