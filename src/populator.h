#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <regex.h>
#include <string.h>
#include <ctype.h>

#include "hash_array.h"
#include "generator.h"

/**
 * @brief Generate the file
 * 
 * @param fd The file descriptor to write to
 */
void generate(FILE* fd);