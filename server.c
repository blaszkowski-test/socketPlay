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
#include "utils.h"

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
    while (1)
    {
        memset((void *) buffer, '\0', sizeof (buffer));
        memset((void *) perclient, '\0', sizeof (perclient));
        switch (read(client_id, buffer, 255))
        {
            case 0: puts("server disconnected");
                return NULL;
            case -1: error("ERROR reading from socket");
            default:
                snprintf(perclient, sizeof (perclient), "From %d: %s", client_id, buffer);
                puts(perclient);
        }
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

int bind_socket(unsigned short port_number)
{
    struct sockaddr_in serv_addr;
    int sockfd = socket(AF_INET, SOCK_STREAM, SOL_TCP);
    if (sockfd < 0)
    {
        error("ERROR opening socket");
    }
    memset((void *) &serv_addr, '\0', sizeof (serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_number);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *) &serv_addr,
            sizeof (serv_addr)) < 0)
    {
        error("ERROR on binding");
    }

    listen(sockfd, 5);

    return sockfd;
}

void server(unsigned short port_number)
{
    initList(&oneList);
    pthread_t thread;
    pthread_mutex_init(&count_mutex, NULL);
    struct sockaddr_in client_settings;
    socklen_t client_settings_length = sizeof (client_settings);
    long accepted_socket_id;

    int sockfd = bind_socket(port_number);

    while (1)
    {
        accepted_socket_id = accept(sockfd, (struct sockaddr *) &client_settings, &client_settings_length);

        debug_sockaddr_in(&client_settings);

        if (accepted_socket_id < 0)
            error("ERROR on accept");

        listPushBack(&oneList, (void*) accepted_socket_id);
        pthread_create(&thread, NULL, one_client, (void*) accepted_socket_id);
        pthread_detach(thread);
    }

    freeList(&oneList, false);
    pthread_mutex_destroy(&count_mutex);
}