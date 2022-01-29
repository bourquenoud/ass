#pragma once

#include "generated/ass.tab.h"

void fail_set_loc(YYLTYPE);
void fail_error(const char*,...);
void fail_warning(const char*, ...);