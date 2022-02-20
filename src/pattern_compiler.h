#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include "state_machine.h"
#include "linked_list.h"

/**
 * @brief Generate an nfa from a regex-like sequence
 * @details Produce a non-deteministic finite automata from
 *          a squence of tokens and control characters. Positive
 *          values are treated as tokens, negative values are
 *          treated as control characters. Control characters
 *          are 2's complement of the regex control characters.
 *          Backslash doesn't escape the next character. -1 is
 *          the only negative value token, and is used as EOF.
 * 
 * @remark  Function is reentrant
 *  
 * @param count Length of the sequence
 * @param sequence The sequence of characters
 * @param output The output value at terminal states
 * @return state_machine_t 
 */
state_machine_t pattern_compiler(size_t count, const int* sequence, int output);