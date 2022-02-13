#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

#include "state_machine.h"
#include "hash_array.h"
#include "dynamic_array.h"

/**
 * @brief Set the file descriptor for the generator
 *
 * @param file_descriptor The file descriptor to which the generator will write
 */
void generator_set_file_descriptor(FILE *file_descriptor);

/**
 * @brief Set the state machine to use for the generation
 * 
 * @param state_machine The state machine to use for the generation
 */
void generator_set_state_machine(state_machine_t* state_machine);

/**
 * @brief Generate the definiton for the state machine
 * 
 * @param indent The indentation level of the whole block
 */
void generator_dfa_def(int indent);

/**
 * @brief Generate an implemention of the state machine using a
 *          large switch-case statement
 * 
 * @param indent The indentation level of the whole block
 */
void generator_dfa_switch(int indent);