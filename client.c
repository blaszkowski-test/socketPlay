#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <unistd.h> 
#include <pthread.h>
#include "error_handler.h"
#include "client.h"
#include "utils.h"

void * client_screen(void * arg)
{
    long server_id = (long) arg;
    char buffer[256];
    int n;
    while (1)
    {
        clear_memory(buffer, sizeof (buffer));
        n = read(server_id, buffer, 255);
        if (n == 0) break;
        if (n < 0)
            error("ERROR reading from socket");
        printf("%s", buffer);
    }
}

void client(char** argv)
{
    pthread_t thread;
    long sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];
    portno = atoi(argv[3]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    server = gethostbyname(argv[2]);

    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }
    printf("name: %s, h_addr_list: %4s, lenght: %d\n", server->h_name, server->h_addr_list[0], server->h_length);

    clear_memory((void *) &serv_addr, sizeof (serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy((void *) server->h_addr,
            (void *) &serv_addr.sin_addr.s_addr,
            server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof (serv_addr)) < 0)
        error("ERROR connecting");


    pthread_create(&thread, NULL, client_screen, (void*) sockfd);
    pthread_detach(thread);

    printf("Please enter the message: ");
    while (1)
    {

        clear_memory(buffer, sizeof (buffer));
        fgets(buffer, 255, stdin);

        n = write(sockfd, buffer, strlen(buffer));
        if (n < 0)
            error("ERROR writing to socket");


        if (strncmp(buffer, "exit", 4) == 0)
        {
            printf("CLOSING SOCKET FROM CLINET server : %d", sockfd);
            shutdown(sockfd, SHUT_RDWR);
            break;
        }

    }
}
