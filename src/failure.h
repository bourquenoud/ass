#pragma once

#include "generated/ass.tab.h"

int fail_get_warning_count(void);
int fail_get_error_count(void);

void fail_set_loc(YYLTYPE);
void fail_error(const char*,...);
void fail_warning(const char*, ...);