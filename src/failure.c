#include "failure.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#define RESET_COLOUR 0
#define DETA_COLOUR 0
#define INFO_COLOUR 94
#define WARN_COLOUR 93
#define ERRO_COLOUR 91

static YYLTYPE loc;

static int showLoc = 0;
static int showColour = 1;

/**
 * @brief Level of verbosity for the logging system
 *
 * @details Levels :
 *              0 -> Supress everything
 *              1 -> Only show errors
 *              2 -> Show errors and warnings (default)
 *              3 -> Show errors, warnings and infos
 *              4 -> Show errors, warnings, infos and details
 */
static int verbositiy = 2;

// Encaplsulated to have a more realiable error system
static int info_count = 0;
int fail_get_info_count()
{
    return info_count;
}

// Encaplsulated to have a more realiable error system
static int warning_count = 0;
int fail_get_warning_count()
{
    return warning_count;
}

// Encaplsulated to have a more realiable error system
static int error_count = 0; // Quite useless if we exit directly after an error
int fail_get_error_count()
{
    return error_count;
}

// Encaplsulated to have a more realiable error system
void fail_set_verbose(int _verbositiy)
{
    verbositiy = _verbositiy;
}

void fail_inc_verbose()
{
    verbositiy = verbositiy >= 4 ? 4 : verbositiy + 1;
}

// Encaplsulated to have a more realiable error system
void fail_set_loc(YYLTYPE location)
{
    loc = location;
}

// Encaplsulated to have a more realiable error system
void fail_show_loc(int _showLoc)
{
    showLoc = _showLoc;
}

// Encaplsulated to have a more realiable error system
void fail_show_colour(int _showColour)
{
    showColour = _showColour;
}

static void set_colour(int colour);

void fail_detail(const char *format, ...)
{
    info_count++;

    if (verbositiy < 4)
        return;

    va_list args;
    va_start(args, format);
    set_colour(DETA_COLOUR);

    if (showLoc)
        fprintf(stderr, "DETAIL line %i : ", loc.first_line);
    else
        fprintf(stderr, "DETAIL : ");

    vfprintf(stderr, format, args);
    set_colour(RESET_COLOUR);
    fputc('\n', stderr);
    va_end(args);
}

void fail_info(const char *format, ...)
{
    info_count++;

    if (verbositiy < 3)
        return;

    va_list args;
    va_start(args, format);
    set_colour(INFO_COLOUR);

    if (showLoc)
        fprintf(stderr, "INFO line %i : ", loc.first_line);
    else
        fprintf(stderr, "INFO : ");
    vfprintf(stderr, format, args);
    set_colour(RESET_COLOUR);
    fputc('\n', stderr);
    va_end(args);
}

void fail_warning(const char *format, ...)
{
    warning_count++;

    if (verbositiy < 2)
        return;

    va_list args;
    va_start(args, format);
    set_colour(WARN_COLOUR);

    if (showLoc)
        fprintf(stderr, "WARNING line %i : ", loc.first_line);
    else
        fprintf(stderr, "WARNING : ");

    vfprintf(stderr, format, args);
    set_colour(RESET_COLOUR);
    fputc('\n', stderr);
    va_end(args);
}

// Write a message before exiting
void fail_error(const char *format, ...)
{
    error_count++;

    if (verbositiy < 1)
        return;

    va_list args;
    va_start(args, format);
    set_colour(ERRO_COLOUR);

    if (showLoc)
        fprintf(stderr, "ERROR line %i : ", loc.first_line);
    else
        fprintf(stderr, "ERROR : ");

    vfprintf(stderr, format, args);
    set_colour(RESET_COLOUR);
    fputc('\n', stderr);
    va_end(args);
    // exit(EXIT_FAILURE);
}

void set_colour(int colour)
{
    if (showColour)
        fprintf(stderr, "\033[%im", colour);
}