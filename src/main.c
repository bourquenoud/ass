#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

#include "generated/ass.tab.h"
#include "commands.h"
#include "hash_array.h"
#include "parameters.h"
#include "failure.h"
#include "bitpattern.h"
#include "dynamic_array.h"
#include "tokeniser.h"
#include "macro.h"
#include "xmalloc.h"
#include "generator.h"
#include "populator.h"
#include "lexer.h"
#include "parser.h"
#include "parser_gen.h"

int main(int argc, char **argv)
{
    char *file_list[argc];
    int file_count = 0;

    for (int i = 1; i < argc; i++)
    {
        file_list[file_count] = argv[i];
        file_count++;
    }

    // Gives repeatable values for the name generation
    srand(1);

    // Init everything for parsing
    command_init();
    param_init();

    // Parse the file and generate all data
    parse_file(file_count, file_list);

    // Fill parameters that have not been set
    param_fill_unset();

    // Generate the lexer
    lexer_init();
    lexer_generate();

    // Generate the parser
    parser_init();
    parser_generate();

    // Generate the file
    FILE *fd = fopen("parser.c", "w");
    generator_set_file_descriptor(fd);
    generate(fd);
    fclose(fd);

    return 0;
}
