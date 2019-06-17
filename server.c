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

static struct List oneList;
static pthread_mutex_t count_mutex;
static int max_server_clients; 
static volatile int current_clients_length;
static const char * to_many_connections = "To Many Connections";

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
    --current_clients_length;
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
            case 0: 
                puts("client disconnected");
                remove_one_client(client_id);
                return NULL;
            case -1: error("ERROR reading from socket");
            default:
                snprintf(perclient, sizeof (perclient), "From %d: %s", client_id, buffer);
                puts(perclient);
        }
        if (strncmp(buffer, "exit\n", 5) == 0)
        {
            printf("CLOSING SOCKET FROM CLIENT : %d", client_id);
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

int server_connections_guard(long accepted_socket_id)
{
    pthread_mutex_lock(&count_mutex);
    int status = 1;
    ++current_clients_length;
    if (current_clients_length > max_server_clients)
    {
        write(accepted_socket_id, to_many_connections, strlen(to_many_connections));
        shutdown(accepted_socket_id, SHUT_RDWR);
        --current_clients_length;
        status = 0;
    }
    pthread_mutex_unlock(&count_mutex);
    return status;
}

void server(unsigned short port_number)
{
    initList(&oneList);
    pthread_t thread;
    max_server_clients = 2;
    current_clients_length = 0;
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

        if (server_connections_guard(accepted_socket_id))
        {
            listPushBack(&oneList, (void*) accepted_socket_id);
            pthread_create(&thread, NULL, one_client, (void*) accepted_socket_id);
            pthread_detach(thread);
        }
    }

    freeList(&oneList, false);
    pthread_mutex_destroy(&count_mutex);
}