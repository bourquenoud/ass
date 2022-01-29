#include "xmalloc.h"

xmalloc_error_handler_t xmalloc_error_handler = xmalloc_default_handler;

//Empty handler
void xmalloc_default_handler(int err){}

void xmalloc_set_handler(xmalloc_error_handler_t handler)
{
    xmalloc_error_handler = handler;
}

void* xmalloc(size_t size)
{
    if(size == 0)
    {
        xmalloc_error_handler(0);
        abort(); //Malloc error
    }

    void* ptr = malloc(size);

    if(ptr == NULL)
    {
        xmalloc_error_handler(1);
        abort(); //Malloc error
    }

    return ptr;
}