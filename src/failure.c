#include "failure.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

static YYLTYPE loc;

//Encaplsulated to have a more realiable error system
static int info_count = 0;
int fail_get_info_count()
{
    return info_count;
}

//Encaplsulated to have a more realiable error system
static int warning_count = 0;
int fail_get_warning_count()
{
    return warning_count;
}

//Encaplsulated to have a more realiable error system
static int error_count = 0; //Quite useless if we exit directly after an error
int fail_get_error_count()
{
    return error_count;
}

void fail_set_loc(YYLTYPE location){
    loc = location;
}

void fail_info(const char* format, ...)
 {
    info_count++;

    va_list args;
    va_start(args, format);
    fprintf(stdout, "\033[34mINFO line %i : ", loc.first_line);
    vfprintf(stdout, format, args);
    fputs("\033[0m\n", stdout);
    va_end(args);
 }

void fail_warning(const char* format, ...)
 {
    warning_count++;

    va_list args;
    va_start(args, format);
    fputs("\033[33m", stderr);
    fprintf(stderr, "WARNING line %i : ", loc.first_line);
    vfprintf(stderr, format, args);
    fputs("\033[0m\n", stderr);
    va_end(args);
 }

//Write a message before exiting
void fail_error(const char *format, ...)
{
    error_count++;

    va_list args;
    va_start(args, format);
    fputs("\033[31m", stderr);
    fprintf(stderr, "ERROR line %i : ", loc.first_line);
    vfprintf(stderr, format, args);
    fputs("\033[0m\n", stderr);
    va_end(args);
    //exit(EXIT_FAILURE);
}