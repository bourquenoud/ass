#pragma once

#include "generated/ass.tab.h"

void fail_set_verbose(int);
void fail_inc_verbose();
void fail_set_loc(YYLTYPE);
void fail_show_loc(int);
void fail_show_colour(int);

int fail_get_info_count(void);
int fail_get_warning_count(void);
int fail_get_error_count(void);

void fail_error(const char *, ...);
void fail_warning(const char *, ...);
void fail_info(const char *, ...);
void fail_debug(const char *, ...);