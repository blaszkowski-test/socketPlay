#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "server.h"
#include "client.h"

/*
 *
 */
int main(int argc, char **argv)
{
    if (argc < 4)
    {
        fprintf(stderr, "usage %s type[server,client] [TCP,UDP] hostname port\n", argv[0]);
        exit(0);
    }

    if (strcmp(argv[1], "server") == 0)
    {
        server(atoi(argv[4]), argv[2]);
    }
    else
    {
        client(argv[2], argv[3], atoi(argv[4]));
    }

    return (EXIT_SUCCESS);
}