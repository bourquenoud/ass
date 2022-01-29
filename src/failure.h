#pragma once

#include "generated/ass.tab.h"

void fail_set_loc(YYLTYPE);

int fail_get_info_count(void);
int fail_get_warning_count(void);
int fail_get_error_count(void);

void fail_error(const char*,...);
void fail_warning(const char*, ...);
void fail_info(const char*, ...);