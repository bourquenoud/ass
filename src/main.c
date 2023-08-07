#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

#include "version.h"
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

char const *const help_message =
    "Usage: %s [OPTION]... -o OUTPUT_FILE INTPUT_FILES\n"
    "This tool can generate an assembler in C\n"
    "based on simple opcode and mnemonics descriptions.\n"
    "It is part of the muonium uEngine project.\n"
    "\n"
    "Options:\n"
    "  -o <FILE>   set the output file\n"
    "  -h          display this help and exit\n"
    "  -V          output version information and exit\n"
    "  -v          set verbosity level to INFOS\n"
    "  -vv         set verbosity level to DETAILS\n"
    "  -W          suppress warnings by setting verbosity level to ERRORS\n"
    "  -s          silent mode\n"
    "  -C          suppress colours\n";

char const *const version_message =
    "ass (Assembly Syntax Synthesiser) %i.%i.%i, build %i-%i\n"
    "Written by Mathieu Bourquenoud.\n"
    "\n"
    "Copyright (C) 2023 Mathieu Bourquenoud.\n"
    "This program comes with ABSOLUTELY NO WARRANTY; not even for\n"
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
    "This is free software, and you are welcome to redistribute it\n"
    "under certain conditions; refer to the license for details.\n";

int main(int argc, char **argv)
{
    // File descriptor for the output file
    // TODO: more meaningful naming
    FILE *fd;
    char *output_file = NULL;
    char *file_list[argc];
    int file_count = 0;
    int opt;
    bool supressed = false;

    // Parse options
    fail_show_loc(false);
    while ((opt = getopt(argc, argv, ":hVsWCvo:")) != -1)
    {
        switch (opt)
        {
        case 'h': // Show help
            printf(help_message, argv[0]);
            exit(EXIT_SUCCESS);
            break;
        case 'V': // Show version
            printf(version_message, VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION, VERSION_BUILD_DATE, VERSION_BUILD);
            exit(EXIT_SUCCESS);
            break;
        case 's': // Silent, suppress all outputs
            supressed = true;
            fail_set_verbose(0);
            break;
        case 'W': // Suppress warnings
            if (supressed)
                break;
            fail_set_verbose(1);
            supressed = true;
            break;
        case 'C': // Suppress the colour
            fail_show_colour(false);
            break;
        case 'v': // Verbose
            if (supressed)
                break;
            fail_inc_verbose();
            break;
        case 'o': // Output file
            if (output_file != NULL)
                fail_warning("Output file path overriden.");
            output_file = optarg;
            fail_debug("Output file is %s", output_file);
            break;
        case ':':
            fail_error("Option '%c' expects an argument", optopt);
            break;
        case '?':
            fail_error("Unknown option '%c'", optopt);
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
            fail_error("%s (%s)", strerror(errno), file_list[file_count]);
        file_count++;
    }

    // Check for previous errors and exit if an error occured during argument parsing
    if (fail_get_error_count() != 0)
    {
        fail_error("Failure during parameter parsing. Exiting.");
        exit(EXIT_FAILURE);
    }

    // Open the output stream, catching errors earlier
    if (output_file == NULL)
        fd = stdout;
    else
        fd = fopen(output_file, "w");

    // Gives repeatable values for the name generation
    srand(1);

    // Init everything for parsing
    command_init();
    param_init();

    // Parse the file and generate all data
    fail_debug("Parsing the file%s", file_count == 1 ? "" : "s");
    parse_file(file_count, file_list);

    // Check for previous errors and exit if an error occured during parsing
    fail_show_loc(false);
    if (fail_get_error_count() != 0)
    {
        fail_error("Failure during file parsing. Exiting.");
        exit(EXIT_FAILURE);
    }

    // Fill parameters that have not been set
    param_fill_unset();

    // Generate the lexer
    fail_debug("Generating the lexer");
    lexer_init();
    lexer_generate();

    // Generate the parser
    fail_debug("Generating the parser");
    parser_init();
    parser_generate();

    // Check for previous errors and exit if an error occured during dfa generation
    if (fail_get_error_count() != 0)
    {
        fail_error("Failure during lexer/parser generation. Exiting.");
        exit(EXIT_FAILURE);
    }

    // Generate the file
    generator_set_file_descriptor(fd);
    generate(fd);
    fclose(fd);

    // Check for previous errors and exit if an error occured during generation
    if (fail_get_error_count() != 0)
    {
        fail_error("Failure during output file generation. Exiting.");
        exit(EXIT_FAILURE);
    }
    else
    {
        fail_info("Success");
        exit(EXIT_SUCCESS);
    }
}