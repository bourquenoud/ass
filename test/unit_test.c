#include <criterion/criterion.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdint-gcc.h>

#include "../src/dynamic_array.h"

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

    //Adding one element to the array
    darray = darray_add(darray, &val);
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

    //Write enough to resize the dynamic array
    sarray[0] = val; //Save the element we added before to the dynamic array for later comparison
    for (int i = 1; i < N_ELEMENTS; i++)
    {
        val = ((uint64_t)rand() << 32) + ((uint64_t)rand());
        darray = darray_add(darray, &val);
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

    //Read and compare all values
    for (int i = 0; i < N_ELEMENTS; i++)
    {
        darray_get(darray, &val, i);
        cr_expect(val == sarray[i], "Failed to read from the dynamic array");
    }

   //Remove 1 element
   darray = darray_remove(darray, 1);
   cr_expect(darray->count==N_ELEMENTS-1, "Failed to remove 1 element");

   //Remove 2 elements
   darray = darray_remove(darray, 2);
   cr_expect(darray->count==N_ELEMENTS-3, "Failed to remove 2 element");

   //Remove 8 elements
   darray = darray_remove(darray, 8);
   cr_expect(darray->count==N_ELEMENTS-11, "Failed to remove 8 element");

   //Remove 19 elements
   darray = darray_remove(darray, 19);
   cr_expect(darray->count==N_ELEMENTS-30, "Failed to remove 19 element");

   //Remove 64 elements
   darray = darray_remove(darray, 64);
   cr_expect(darray->count==N_ELEMENTS-94, "Failed to remove 64 element");

   //Empty the array
   while(darray->count != 0)
   {
       darray = darray_remove(darray, 1);
   }

   cr_expect(darray->count == 0, "Failed to empty the dynamic array");

   //Try to remove from an emtpy array
   darray = darray_remove(darray, 1);

   cr_expect(darray->count == 0, "Underflow protection not working for 1");

   //Try to remove from an emtpy array
   darray = darray_remove(darray, 32);

   cr_expect(darray->count == 0, "Underflow protection not working for 32");
}

Test(default_suite, should_pass)
{
    cr_expect(1 == 1, "This test should always pass. Something is wrong");
}