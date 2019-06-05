#include <stdio.h>
#include <stdlib.h>
#include "error_handler.h"

void error(const char *msg)
{
    perror(msg);
    exit(0);
}