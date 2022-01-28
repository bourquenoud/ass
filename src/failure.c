#include "failure.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

//Write a message before exiting
void fail(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    exit(EXIT_FAILURE);
}