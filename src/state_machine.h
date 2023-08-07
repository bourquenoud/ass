#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

#include "macro.h"
#include "xmalloc.h"
#include "dynamic_array.h"
#include "hash_array.h"

#define CONDITION_EOF -1
#define CONDITION_ANY -2

typedef struct state_s state_t;

typedef struct
{
    int condition; // -2 means any condition
    int next_state_id;
} transistion_t;

struct state_s
{
    int id;
    int end_state;
    int output;
    darray_t *transitions_ttrans;
};

typedef struct
{
    darray_t *states_tstate;
} state_machine_t;

/**
 * @brief Get a state from the state machine
 *
 * @param state_machine The state machine
 * @param id The id of the state to get
 * @return state_t* The state
 */
state_t *state_machine_get_by_id(state_machine_t *state_machine, int id);

/**
 * @brief Add a new state to the state machine
 *
 * @param state_machine The state machine
 * @param end_state If the state is an end state
 * @return state_t* The new state
 */
state_t *state_machine_add_state(state_machine_t *state_machine, int end_state);

/**
 * @brief Initialise the state machine
 *
 * @return state_machine_t The new state machine
 */
state_machine_t state_machine_init();

/**
 * @brief Merge two state machines
 *
 * @param state_machine_a The first state machine
 * @param state_machine_b The second state machine
 * @return state_machine_t The merged state machine
 */
state_machine_t state_machine_merge(state_machine_t *state_machine_a, state_machine_t *state_machine_b);

/**
 * @brief Get a state from the state machine
 * @note May be a duplicate of state_machine_get_by_id
 *
 * @param state_machine The state machine
 * @param state_id The id of the state to get
 * @return state_t* The state
 */
state_t *state_machine_get_state(state_machine_t *state_machine, int state_id);

/**
 * @brief Compare two states
 *
 * @param s1 The first state
 * @param s2 The second state
 * @return true If the states are equal
 * @return false If the states are not equal
 */
bool state_compare_states(state_t *s1, state_t *s2);

/**
 * @brief Compare the transitions of two states
 *
 * @param t1 The first transition list
 * @param t2 The second transition list
 * @return true If the transitions are equal
 * @return false If the transitions are not equal
 */
bool state_compare_transitions(transistion_t *t1, transistion_t *t2);

/**
 * @brief Reduce the state machine
 * @details The state machine is reduced by removing the states that can be merged together. They
 *   are considered mergeable if they have the same output and the same transitions.
 *
 * @param state_machine The state machine
 */
void state_machine_reduce(state_machine_t *state_machine);

/**
 * @brief Change the id of a state
 *
 * @param state_machine The state machine
 * @param new_id The new id
 * @param old_id The old id
 */
void state_machine_replace_id(state_machine_t *state_machine, int new_id, int old_id);

/**
 * @brief Get the list of transitions from a state
 *
 * @param state_machine The state machine
 * @param state_id The id of the state to get the transitions from
 * @return transistion_t*
 */
transistion_t *state_machine_get_transitions(state_machine_t *state_machine, int state_id);

/**
 * @brief Convert a non-deterministic finite state machine to a deterministic finite state machine
 *
 * @param nfa The non-deterministic finite state machine
 * @return state_machine_t The equivalent deterministic finite state machine
 */
state_machine_t state_machine_make_deterministic(state_machine_t *nfa);

/**
 * @brief Remove a state from the state machine
 *
 * @param state_machine The state machine
 */
void state_machine_remove_state(state_machine_t *state_machine);

/**
 * @brief Initialise a state
 *
 * @param id The id of the state
 * @return state_t The initialised state
 */
state_t state_init_state(int id);

/**
 * @brief Add a transition to a state
 *
 * @param _arg_count The number of arguments
 * @param state The state to add the transition to
 * @param next_state_id The id the transition points to
 * @param ... The conditions of the transition (as integers)
 * @return int Always 0
 */
int _state_add_transition(int _arg_count, state_t *state, int next_state_id, ...);

#define state_add_transition(...) _state_add_transition((PP_NARG(__VA_ARGS__)) - 2, __VA_ARGS__)

/**
 * @brief Print a state machine as a graphviz file
 * @note The graphviz file will be overwritten
 * @attention Debug function
 *
 * @param state_machine The state machine
 * @param file_descriptor The file descriptor to print to
 */
void state_machine_print(state_machine_t *state_machine, FILE *file_descriptor);

/**
 * @brief Print a state machine as a graphviz file
 * @note The graphviz file will be overwritten
 * @attention Debug function
 *
 * @param state_machine The state machine
 * @param file_descriptor The file descriptor to print to
 */
void state_machine_print_char(state_machine_t *state_machine, FILE *file_descriptor);
