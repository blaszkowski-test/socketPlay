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
#include <signal.h> 
#include "error_handler.h"
#include "list.h"
#include "server.h"

struct List oneList;
pthread_mutex_t count_mutex;

void remove_one_client(long client_id)
{
    pthread_mutex_lock(&count_mutex);
    int current = 0;
    int index = 0;
    while (listHasNext(&oneList) != false)
    {
        current = (long) listNext(&oneList);
        if (current == client_id)
        {
            listRemove(&oneList, index);
            break;
        }
        ++index;
    }
    resetIterator(&oneList);
    pthread_mutex_unlock(&count_mutex);
}

void send_to_all_clients(char * message, long client_id)
{

    pthread_mutex_lock(&count_mutex);
    int current = 0;
    while (listHasNext(&oneList) != false)
    {
        current = (long) listNext(&oneList);
        if (current == client_id) continue;
        write(current, message, strlen(message));
    }
    resetIterator(&oneList);
    pthread_mutex_unlock(&count_mutex);
}

void *one_client(void *arg)
{
    long client_id = (long) arg;
    char buffer[256];
    char perclient[256];
    int n;
    while (1)
    {
        memset((void *) buffer, '\0', sizeof (buffer));
        memset((void *) perclient, '\0', sizeof (perclient));
        n = read(client_id, buffer, 255);
        if (n == 0) break;
        if (n < 0) error("ERROR reading from socket");
        snprintf(perclient, sizeof (perclient), "From %d: %s", client_id, buffer);
        puts(perclient);

        if (strncmp(buffer, "exit\n", 5) == 0)
        {
            printf("CLOSING SOCKET FROM SERVER clinet : %d", client_id);
            remove_one_client(client_id);
            shutdown(client_id, SHUT_RDWR);
            return NULL;
        }

        send_to_all_clients(perclient, client_id);
    }
    return NULL;
}

int bind_socket(char** argv)
{
    struct sockaddr_in serv_addr;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    memset((void *) &serv_addr, '\0', sizeof (serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[3]));
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *) &serv_addr,
            sizeof (serv_addr)) < 0)
    {
        error("ERROR on binding");
    }
    listen(sockfd, 5);

    return sockfd;
}

void server(char** argv)
{
    initList(&oneList);
    pthread_t thread;
    pthread_mutex_init(&count_mutex, NULL);
    struct sockaddr_in cli_addr;
    int clilen;
    long newsockfd;

    int sockfd = bind_socket(argv);

    while (1)
    {
        clilen = sizeof (cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        printf("sin_addr: %d, sin_family: %d, sin_port: %d\n", htonl(cli_addr.sin_addr.s_addr), cli_addr.sin_family, cli_addr.sin_port);

        if (newsockfd < 0)
            error("ERROR on accept");
        listPushBack(&oneList, (void*) newsockfd);
        pthread_create(&thread, NULL, one_client, (void*) newsockfd);
        pthread_detach(thread);
    }

    freeList(&oneList, false);
    pthread_mutex_destroy(&count_mutex);
}