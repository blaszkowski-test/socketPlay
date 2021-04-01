#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "error_handler.h"
#include "list.h"
#include "server.h"
#include "utils.h"
#include <errno.h>

static struct List oneList;
static pthread_mutex_t count_mutex;
static int max_server_clients;
static volatile int current_clients_length;
static const char *to_many_connections = "To Many Connections";
static sig_atomic_t server_run = 1;
void server_run_handler(int param)
{
    perror("Interrupt bye");
    server_run = 0;
}

void remove_one_client(long client_id)
{
    pthread_mutex_lock(&count_mutex);
    int current = 0;
    int index = 0;
    while (listHasNext(&oneList) != false)
    {
        current = (long)listNext(&oneList);
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

void send_to_all_clients_tcp(char *message, long client_id)
{
    pthread_mutex_lock(&count_mutex);
    int current = 0;
    while (listHasNext(&oneList) != false)
    {
        current = (long)listNext(&oneList);
        if (current == client_id)
            continue;
        write(current, message, strlen(message));
    }
    resetIterator(&oneList);
    pthread_mutex_unlock(&count_mutex);
}

void send_to_all_clients_udp(int sockfd, const char *message, struct sockaddr_in *client)
{
    struct sockaddr_in *current = 0;
    int client_exists = false;
    ssize_t message_length = 0;
    int index = 0;
    while (listHasNext(&oneList) != false)
    {
        current = (struct sockaddr_in *)listNext(&oneList);
        if (current->sin_addr.s_addr == client->sin_addr.s_addr)
            continue;

        message_length = sendto(sockfd, message, strlen(message),
                                0, (const struct sockaddr *)current,
                                sizeof(struct sockaddr_in));

        // if (message_length < 1)
        // {
        //     listRemove(&oneList, index);
        //     free(current);
        //     resetIterator(&oneList);
        //     index = -1;
        // }
        // ++index;
    }
    resetIterator(&oneList);
}

void *one_client(void *arg)
{
    long client_id = (long)arg;
    char buffer[256];
    char perclient[256];
    while (1)
    {
        memset((void *)buffer, '\0', sizeof(buffer));
        memset((void *)perclient, '\0', sizeof(perclient));
        switch (read(client_id, buffer, 255))
        {
        case 0:
            puts("client disconnected");
            remove_one_client(client_id);
            return NULL;
        case -1:
            error("ERROR reading from socket");
        default:
            snprintf(perclient, sizeof(perclient), "From %d: %s", client_id, buffer);
            puts(perclient);
        }
        if (strncmp(buffer, "exit\n", 5) == 0)
        {
            printf("CLOSING SOCKET FROM CLIENT : %d", client_id);
            remove_one_client(client_id);
            shutdown(client_id, SHUT_RDWR);
            return NULL;
        }

        send_to_all_clients_tcp(perclient, client_id);
    }
    return NULL;
}

int bind_socket_tcp(unsigned short port_number)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, SOL_TCP);
    if (sockfd < 0)
    {
        error("ERROR opening socket");
    }

    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        error("setsockopt");
    }

    struct sockaddr_in serv_addr;
    memset((void *)&serv_addr, '\0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_number);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        error("ERROR on binding");
    }
    if (listen(sockfd, 5) < 0)
    {
        error("ERROR on listen");
    }

    return sockfd;
}

int bind_socket_udp(unsigned short port_number)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, SOL_UDP);
    if (sockfd < 0)
    {
        error("ERROR opening socket");
    }

    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        error("setsockopt");
    }

    struct sockaddr_in serv_addr;
    memset((void *)&serv_addr, '\0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_number);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        error("ERROR on binding");
    }

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

void udp_push_list(struct sockaddr_in *client)
{
    struct sockaddr_in *current = 0;
    int client_exists = false;
    while (listHasNext(&oneList) != false)
    {
        current = (struct sockaddr_in *)listNext(&oneList);
        if (current->sin_addr.s_addr == client->sin_addr.s_addr)
        {
            client_exists = true;
            break;
        }
    }
    resetIterator(&oneList);

    if (false == client_exists)
    {
        struct sockaddr_in *client_buffer =
            (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
        memcpy(client_buffer, client, sizeof(struct sockaddr_in));

        listPushBack(&oneList, client_buffer);
    }
}

void server(unsigned short port_number, const char *type)
{
    void (*prev_handler)(int);
    prev_handler = signal(SIGINT | SIGTERM | SIGABRT, server_run_handler);

    struct timeval timeout;
    timeout.tv_sec = 1; // 1s timeout
    timeout.tv_usec = 0;

    char buffer[256];

    initList(&oneList);
    pthread_t thread;
    max_server_clients = 2;
    current_clients_length = 0;
    pthread_mutex_init(&count_mutex, NULL);
    struct sockaddr_in client_settings;
    socklen_t client_settings_length = sizeof(client_settings);
    long accepted_socket_id;
    int select_status;
    ssize_t message_length = 0;
    int sockfd = 0;
    if (0 == strcmp("TCP", type))
    {
        sockfd = bind_socket_tcp(port_number);
    }
    else
    {
        sockfd = bind_socket_udp(port_number);
    }

    fd_set main_fds;
    FD_ZERO(&main_fds);
    FD_SET(sockfd, &main_fds);

    while (server_run)
    {
        if (socket_can_read(main_fds, sockfd, &timeout))
        {
            if (0 == strcmp("TCP", type))
            {
                accepted_socket_id = accept(sockfd, (struct sockaddr *)&client_settings, &client_settings_length);
                debug_sockaddr_in(&client_settings);

                if (accepted_socket_id < 0)
                    error("ERROR on accept");

                if (server_connections_guard(accepted_socket_id))
                {
                    listPushBack(&oneList, (void *)accepted_socket_id);
                    pthread_create(&thread, NULL, one_client, (void *)accepted_socket_id);
                    pthread_detach(thread);
                }
            }
            else
            {
                memset((void *)buffer, '\0', sizeof(buffer));
                message_length = recvfrom(sockfd, (char *)buffer, 255,
                                          0, (struct sockaddr *)&client_settings,
                                          &client_settings_length);
                if (message_length > 0)
                {
                    puts(buffer);
                    udp_push_list(&client_settings);
                    send_to_all_clients_udp(sockfd, buffer, &client_settings);
                }
                else if (-1 == message_length)
                {
                    error(strerror(errno));                    
                }
            }
        }
    }
    if (0 == strcmp("TCP", type))
    {
        freeList(&oneList, false);
    }
    else
    {
        freeList(&oneList, true);
    }
    pthread_mutex_destroy(&count_mutex);
}