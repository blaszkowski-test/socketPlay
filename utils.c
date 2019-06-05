#include <utime.h>
#include <string.h>

void clear_memory(void * pointer, size_t size)
{
    memset(pointer, '\0', size);
}