#pragma once

#include <stdlib.h>
#include <stdint.h>

#include "state_machine.h"
#include "linked_list.h"

state_machine_t tokeniser_array_to_nfa(int count, const char const **string_array);
state_machine_t tokeniser_str_to_nfa(const char *string);
