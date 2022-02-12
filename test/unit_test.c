#include <criterion/criterion.h>
#include <criterion/redirect.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdint-gcc.h>
#include <time.h>

#include "../src/dynamic_array.h"
#include "../src/state_machine.h"

// TODO: make better tests

void redirect_all_stdout(void)
{
    cr_redirect_stdout();
    cr_redirect_stderr();
}

TestSuite(default_suite);

Test(default_suite, dynamic_array)
{
    const int N_ELEMENTS = 128;
    const int LAST_ELEMENT = (N_ELEMENTS - 1) * sizeof(uint64_t);

    uint64_t sarray[N_ELEMENTS];

    srand(0);

    uint64_t val = ((uint64_t)rand() << 32) + ((uint64_t)rand());

    darray_t *darray = darray_init(sizeof(uint64_t));
    cr_expect(darray != NULL, "Failed to create the dynamic array");

    // Adding one element to the array
    darray_add(&darray, val);
    cr_expect(
        ((uint64_t)darray->element_list[0] |
         (uint64_t)darray->element_list[1] << 8 |
         (uint64_t)darray->element_list[2] << 16 |
         (uint64_t)darray->element_list[3] << 24 |
         (uint64_t)darray->element_list[4] << 32 |
         (uint64_t)darray->element_list[5] << 40 |
         (uint64_t)darray->element_list[6] << 48 |
         (uint64_t)darray->element_list[7] << 56) == val,
        "Failed to write one value to the dynamic array.");

    // Write enough to resize the dynamic array
    sarray[0] = val; // Save the element we added before to the dynamic array for later comparison
    for (int i = 1; i < N_ELEMENTS; i++)
    {
        val = ((uint64_t)rand() << 32) + ((uint64_t)rand());
        darray_add(&darray, val);
        sarray[i] = val;
    }
    cr_expect(
        ((uint64_t)darray->element_list[LAST_ELEMENT] |
         (uint64_t)darray->element_list[LAST_ELEMENT + 1] << 8 |
         (uint64_t)darray->element_list[LAST_ELEMENT + 2] << 16 |
         (uint64_t)darray->element_list[LAST_ELEMENT + 3] << 24 |
         (uint64_t)darray->element_list[LAST_ELEMENT + 4] << 32 |
         (uint64_t)darray->element_list[LAST_ELEMENT + 5] << 40 |
         (uint64_t)darray->element_list[LAST_ELEMENT + 6] << 48 |
         (uint64_t)darray->element_list[LAST_ELEMENT + 7] << 56) == val,
        "Failed to write multiple values to the dynamic array.");

    // Read and compare all values
    for (int i = 0; i < N_ELEMENTS; i++)
    {
        darray_get(&darray, &val, i);
        cr_expect(val == sarray[i], "Failed to read from the dynamic array using 'darray_get'");
        cr_expect(*((uint64_t *)(darray_get_ptr(&darray, i))) == sarray[i], "Failed to read from the dynamic array using 'darray_get_at'");
    }

    // Copy the array
    darray_t *copied_darray = darray_init(darray->element_size);
    darray_copy(&copied_darray, &darray);

    for (int i = 0; i < N_ELEMENTS; i++)
    {
        cr_expect(*((uint64_t *)(darray_get_ptr(&copied_darray, i))) == sarray[i], "Array copy failed, got %#x expected %#x", *((uint64_t *)(darray_get_ptr(&copied_darray, i))), sarray[i]);
    }

    // Remove 1 element
    darray_remove(&darray, 1);
    cr_expect(darray->count == N_ELEMENTS - 1, "Failed to remove 1 element");

    // Remove 2 elements
    darray_remove(&darray, 2);
    cr_expect(darray->count == N_ELEMENTS - 3, "Failed to remove 2 element");

    // Remove 8 elements
    darray_remove(&darray, 8);
    cr_expect(darray->count == N_ELEMENTS - 11, "Failed to remove 8 element");

    // Remove 19 elements
    darray_remove(&darray, 19);
    cr_expect(darray->count == N_ELEMENTS - 30, "Failed to remove 19 element");

    // Remove 64 elements
    darray_remove(&darray, 64);
    cr_expect(darray->count == N_ELEMENTS - 94, "Failed to remove 64 element");

    // Remove the 5th element
    darray_remove_at(&darray, 1, 5);
    cr_expect(darray->count == N_ELEMENTS - 95, "Failed to remove the 5th element");
    cr_expect(*((uint64_t *)darray_get_ptr(&darray, 4)) == sarray[4], "4th element not matching");
    cr_expect(*((uint64_t *)darray_get_ptr(&darray, 5)) == sarray[6], "new 5th element not matching");

    // Empty the array
    while (darray->count != 0)
    {
        darray_remove(&darray, 1);
    }

    cr_expect(darray->count == 0, "Failed to empty the dynamic array");

    // Try to remove from an emtpy array
    darray_remove(&darray, 1);

    cr_expect(darray->count == 0, "Underflow protection not working for 1");

    // Try to remove from an emtpy array
    darray_remove(&darray, 32);

    cr_expect(darray->count == 0, "Underflow protection not working for 32");
}

/* //Test disabled for now because the transition data structure changed
Test(default_suite, transition)
{
    transistion_t t1;
    transistion_t t2;
    transistion_t t3;
    t1.conditions_tint = darray_init(sizeof(int));
    t1.next_state_id = 1;
    t2.conditions_tint = darray_init(sizeof(int));
    t2.next_state_id = 1;
    t3.conditions_tint = darray_init(sizeof(int));
    t3.next_state_id = 1;

    const int const vals[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

    darray_add(&(t1.conditions_tint), vals[0]);
    darray_add(&(t1.conditions_tint), vals[1]);
    darray_add(&(t1.conditions_tint), vals[2]);
    darray_add(&(t1.conditions_tint), vals[3]);

    darray_add(&(t2.conditions_tint), vals[0]);
    darray_add(&(t2.conditions_tint), vals[1]);
    darray_add(&(t2.conditions_tint), vals[2]);
    darray_add(&(t2.conditions_tint), vals[3]);

    cr_expect(state_compare_transitions(&t1, &t2) == true, "Comparison should return true");

    t2.next_state_id = 2;

    cr_expect(state_compare_transitions(&t1, &t2) == false, "Comparison should return false");

    darray_add(&(t3.conditions_tint), vals[2]);
    darray_add(&(t3.conditions_tint), vals[1]);
    darray_add(&(t3.conditions_tint), vals[3]);
    darray_add(&(t3.conditions_tint), vals[8]);

    cr_expect(state_compare_transitions(&t1, &t3) == false, "Comparison should return false");
}*/

Test(default_suite, state)
{
    const int N_STATE = 16;
    state_t state_array[N_STATE];

    // Init all states
    for (int i = 0; i < N_STATE; i++)
    {
        state_array[i] = state_init_state(i);
        cr_expect(state_array[i].transitions_ttrans != NULL, "Unable to initialise the state n%i", i);
    }
}

Test(default_suite, state_machine)
{
    // Create a state machine
    state_machine_t state_machine = state_machine_init();
    cr_expect(state_machine.states_tstate != NULL, "Failed to init the state machine");
    cr_expect(((state_t *)(state_machine.states_tstate->element_list))->id == 0, "Failed to add the default state");

    state_t *new_state = state_machine_add_state(&state_machine, 0);
    cr_expect(((state_t *)(state_machine.states_tstate->element_list) + 1)->id == 1, "Failed to add a new state");
    cr_expect(((state_t *)(state_machine.states_tstate->element_list) + 1)->id == new_state->id, "Returned state pointer doesn't match with the storred value");

    new_state->id = 78;
    cr_expect(((state_t *)(state_machine.states_tstate->element_list) + 1)->id == 78, "Failed to change the id of the state");
}

state_machine_t state_machine_1, state_machine_2;
void init_state_machine(void)
{
    // Create a state machine
    state_machine_1 = state_machine_init();
    cr_expect(state_machine_1.states_tstate != NULL, "State list not initialised.");

    // Generate multiple states with transitions
    state_t *current_state = state_machine_get_state(&state_machine_1, 0);
    state_add_transition(current_state, 1, 1);
    state_add_transition(current_state, 8, 0);
    state_add_transition(current_state, 7, 0);

    current_state = state_machine_add_state(&state_machine_1, 0); // ID 1
    state_add_transition(current_state, 2, 1);
    state_add_transition(current_state, 10, 1);
    state_add_transition(current_state, 5, 0);

    current_state = state_machine_add_state(&state_machine_1, 0); // ID 2
    state_add_transition(current_state, 3, 1);
    state_add_transition(current_state, 4, 0);
    state_add_transition(current_state, 1, 0);

    current_state = state_machine_add_state(&state_machine_1, 0); // ID 3
    state_add_transition(current_state, 4, 1);
    state_add_transition(current_state, 3, 0);

    current_state = state_machine_add_state(&state_machine_1, 0); // ID 4
    state_add_transition(current_state, 5, 1);
    state_add_transition(current_state, 6, 0);

    current_state = state_machine_add_state(&state_machine_1, 0); // ID 5
    state_add_transition(current_state, 6, 1);
    state_add_transition(current_state, 6, 0);

    current_state = state_machine_add_state(&state_machine_1, 0); // ID 6
    state_add_transition(current_state, 2, 1);

    current_state = state_machine_add_state(&state_machine_1, 0); // ID 7
    state_add_transition(current_state, 8, 1);
    state_add_transition(current_state, 9, 0);

    current_state = state_machine_add_state(&state_machine_1, 0); // ID 8
    state_add_transition(current_state, 5, 1);

    current_state = state_machine_add_state(&state_machine_1, 0); // ID 9
    state_add_transition(current_state, 6, 1);
    state_add_transition(current_state, 6, 0);

    current_state = state_machine_add_state(&state_machine_1, 1); // ID 10
    state_add_transition(current_state, 10, 1);
    state_add_transition(current_state, 8, 1);

    // Create a second state machine
    state_machine_2 = state_machine_init();
    cr_expect(state_machine_2.states_tstate != NULL, "State list not initialised.");

    // Generate multiple states with transitions
    current_state = state_machine_get_state(&state_machine_2, 0);
    state_add_transition(current_state, 1, 1);
    state_add_transition(current_state, 3, 0);

    current_state = state_machine_add_state(&state_machine_2, 0); // ID 1
    state_add_transition(current_state, 2, 1);
    state_add_transition(current_state, 1, 1);
    state_add_transition(current_state, 3, 0);
    state_add_transition(current_state, 1, 0);

    current_state = state_machine_add_state(&state_machine_2, 0); // ID 2
    state_add_transition(current_state, 3, 1);
    state_add_transition(current_state, 0, 0);

    current_state = state_machine_add_state(&state_machine_2, 0); // ID 3
    state_add_transition(current_state, 2, 1);
    state_add_transition(current_state, 3, 0);
    state_add_transition(current_state, 1, 0);
}

TestSuite(state_machine_suite, .init = init_state_machine);

// TODO: THis test doesn't test anything actually
Test(state_machine_suite, state_machine_merge)
{
    printf("PRINTING STATE MACHINE\n");
    state_machine_print(&state_machine_1, stdout);
    printf("DONE PRITING STATE MACHINE\n");

    printf("PRINTING STATE MACHINE\n");
    state_machine_print(&state_machine_2, stdout);
    printf("DONE PRITING STATE MACHINE\n");

    state_machine_t merged_state_machine = state_machine_merge(&state_machine_1, &state_machine_2);

    printf("PRINTING STATE MACHINE\n");
    state_machine_print(&merged_state_machine, stdout);
    printf("DONE PRITING STATE MACHINE\n");
}

Test(state_machine_suite, state_machine_reduce)
{
    state_machine_reduce(&state_machine_1);

    FILE *fd = fopen("graph.dot", "w");

    printf("PRINTING STATE MACHINE\n");
    state_machine_print(&state_machine_1, fd);
    printf("DONE PRITING STATE MACHINE\n");

    fclose(fd);
}

Test(default_suite, should_pass)
{
    cr_expect(1 == 1, "This test should always pass. Something is wrong");
}