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
    // File descriptor for the output file
    // TODO: more meaningful naming
    FILE *fd;
    char *output_file = NULL;
    char *file_list[argc];
    int file_count = 0;
    int opt;

    // Parse options
    while ((opt = getopt(argc, argv, ":o:v")) != -1)
    {
        switch (opt)
        {
        case 'v': // Verbose
            fail_set_verbose(3);
        case 'o': // Output file
            printf("Setting output file as %s\n", optarg);
            if (output_file != NULL)
                printf("WARNING : Output file path overriden.\n");
            output_file = optarg;
            break;
        case ':':
            printf("ERROR : Option '%c' expects an argument\n", opt);
            exit(EXIT_FAILURE);
            break;
        case '?':
            printf("ERROR : Unknown option '%c'\n", optopt);
            exit(EXIT_FAILURE);
            break;
        default:
            abort();
        }
    }

    // Non-option arguments (parsed as input files)
    bool failed = false;
    for (int i = optind; i < argc; i++)
    {
        file_list[file_count] = argv[i];
        // Check if we can read it before
        if (access(file_list[file_count], R_OK) != 0)
        {
            printf("ERROR : %s (%s)\n", strerror(errno), file_list[file_count]);
            failed = true;
        }
        file_count++;
    }

    if (failed)
        exit(EXIT_FAILURE);
    
    if (output_file == NULL)
        fd = stdin;
    else
        fd = fopen(output_file, "w");

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
    generator_set_file_descriptor(fd);
    generate(fd);
    fclose(fd);

    return 0;
}
