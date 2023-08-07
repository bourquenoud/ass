#pragma once

#include <stdlib.h>
#include <stdint.h>

void xmalloc_default_handler(int err);

typedef void (*xmalloc_error_handler_t)(int);

void xmalloc_set_handler(xmalloc_error_handler_t handler);
void *xmalloc(size_t size);
