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
#include <arpa/inet.h>
#include <errno.h>

static struct sockaddr_in server_settings;
static struct timeval timeout;

void *client_screen_tcp(void *arg)
{
    long server_socket = (long)arg;
    char buffer[256];
    fd_set main_fds;
    FD_ZERO(&main_fds);
    FD_SET(server_socket, &main_fds);
    while (1)
    {
        errno = 0;
        while (!socket_can_read(main_fds, server_socket, &timeout))
        {
            if (errno > 0)
            {
                perror("socket read");
                break;
            }
        }

        clear_memory(buffer, sizeof(buffer));
        switch (read(server_socket, buffer, 255))
        {
        case 0:
            error("Server Disconnected");
        case -1:
            error("ERROR reading from socket");
        default:
            printf("%s", buffer);
        }
    }
    return NULL;
}

void *client_screen_udp(void *arg)
{
    long server_socket = (long)arg;
    char buffer[256];
    socklen_t server_settings_length = sizeof(server_settings);

    ssize_t message_length = 0;
    fd_set main_fds;
    FD_ZERO(&main_fds);
    FD_SET(server_socket, &main_fds);
    while (1)
    {
        errno = 0;
        while (!socket_can_read(main_fds, server_socket, &timeout))
        {
            if (errno > 0)
            {
                perror("socket read");
                break;
            }
        }

        clear_memory(buffer, sizeof(buffer));

        message_length = recvfrom(server_socket, (char *)buffer, 255,
                                  0, (struct sockaddr *)&server_settings,
                                  &server_settings_length);

        switch (message_length)
        {
        case 0:
            error("server disconnected");
        case -1:
            error("ERROR reading from socket");
        default:
            printf("%s", buffer);
        }
    }
    return NULL;
}

void setup_server_settings(struct hostent *connection_host, unsigned short port_number)
{
    server_settings.sin_family = AF_INET;
    server_settings.sin_addr.s_addr = **(unsigned int **)connection_host->h_addr_list; // when changed to BigEndian with htonl, doesn't work, so leave as LittleEndian
    server_settings.sin_port = htons(port_number);
}

long connect_to_server_tcp(const char *host_name, unsigned short port_number)
{
    long server_socket;

    struct hostent *connection_host;

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        error("ERROR opening socket");
    }
    if ((connection_host = gethostbyname(host_name)) == NULL)
    {
        error("ERROR, no such host\n");
    }
    debug_hostent(connection_host);
    setup_server_settings(connection_host, port_number);
    if (connect(server_socket, (struct sockaddr *)&server_settings, sizeof(server_settings)) < 0)
    {
        error("ERROR connecting");
    }

    return server_socket;
}

long connect_to_server_udp(const char *host_name, unsigned short port_number)
{
    long server_socket;

    struct hostent *connection_host;

    if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        error("ERROR opening socket");
    }
    if ((connection_host = gethostbyname(host_name)) == NULL)
    {
        error("ERROR, no such host\n");
    }
    debug_hostent(connection_host);
    setup_server_settings(connection_host, port_number);

    return server_socket;
}

void create_read_socket_thread(long server_socket, const char *type)
{
    pthread_t thread;
    if (0 == strcmp("TCP", type))
    {
        pthread_create(&thread, NULL, client_screen_tcp, (void *)server_socket);
    }
    else
    {
        pthread_create(&thread, NULL, client_screen_udp, (void *)server_socket);
    }
    pthread_detach(thread);
}

void client(const char *type, const char *host_name, unsigned short port_number)
{
    clear_memory((void *)&server_settings, sizeof(server_settings));

    timeout.tv_sec = 1; // 1s timeout
    timeout.tv_usec = 0;
    long server_socket;
    char buffer[256];
    ssize_t message_length = 0;

    if (0 == strcmp("TCP", type))
    {
        server_socket = connect_to_server_tcp(host_name, port_number);
    }
    else
    {
        server_socket = connect_to_server_udp(host_name, port_number);
    }

    create_read_socket_thread(server_socket, type);

    fd_set main_fds;
    FD_ZERO(&main_fds);
    FD_SET(server_socket, &main_fds);

    while (1)
    {
        puts("You: \n");
        clear_memory(buffer, sizeof(buffer));
        fgets(buffer, 255, stdin);
        // strcpy(buffer, "DUPA");

        errno = 0;
        while (!socket_can_write(main_fds, server_socket, &timeout))
        {
            if (errno > 0)
            {
                perror("socket write");
                continue;
            }
        }

        if (0 == strcmp("TCP", type))
        {
            if (write(server_socket, buffer, strlen(buffer)) < 0)
                error("ERROR writing to socket");
        }
        else
        {
            message_length = sendto(server_socket, buffer, strlen(buffer),
                                    0, (const struct sockaddr *)&server_settings,
                                    sizeof(struct sockaddr_in));
            printf("bytes sent: %ld", message_length);
        }

        if (strncmp(buffer, "exit\n", 5) == 0)
        {
            printf("CLOSING SOCKET FROM CLINET connection_host : %ld\n", server_socket);
            shutdown(server_socket, SHUT_RDWR);
            break;
        }
    }
}
