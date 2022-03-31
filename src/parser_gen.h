#pragma once

#include "generator.h"
#include "lexer.h"

/**
 * @brief Initialise the parser
 * 
 */
void parser_init();

/**
 * @brief Generate the parser from the list of rule
 * 
 */
void parser_generate();