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

void * client_screen(void * arg)
{
    long server_socket = (long) arg;
    char buffer[256];
    while (1)
    {
        clear_memory(buffer, sizeof (buffer));
        switch (read(server_socket, buffer, 255))
        {
            case 0: puts("server disconnected");
                return NULL;
            case -1: error("ERROR reading from socket");
            default: printf("%s", buffer);
        }
    }
    return NULL;
}

void setup_server_settings(struct sockaddr_in * server_settings, struct hostent *connection_host, unsigned short port_number)
{
    server_settings->sin_family = AF_INET;
    server_settings->sin_addr.s_addr = **(unsigned int **) connection_host->h_addr_list; // when changed to BigEndian with htonl, doesn't work, so leave as LittleEndian
    server_settings->sin_port = htons(port_number);
}

long connect_to_server(const char* host_name, unsigned short port_number)
{
    long server_socket;
    struct sockaddr_in server_settings;
    clear_memory((void *) &server_settings, sizeof (server_settings));
    struct hostent *connection_host;

    if ((server_socket = socket(AF_INET, SOCK_STREAM, SOL_TCP)) < 0)
    {
        error("ERROR opening socket");
    }
    if ((connection_host = gethostbyname(host_name)) == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }
    debug_hostent(connection_host);
    setup_server_settings(&server_settings, connection_host, port_number);

    if (connect(server_socket, (struct sockaddr *) &server_settings, sizeof (server_settings)) < 0)
        error("ERROR connecting");

    return server_socket;

}

void create_read_socket_thread(long server_socket)
{
    pthread_t thread;
    pthread_create(&thread, NULL, client_screen, (void*) server_socket);
    pthread_detach(thread);
}

void client(const char* host_name, unsigned short port_number)
{
    long server_socket;
    char buffer[256];

    server_socket = connect_to_server(host_name, port_number);

    create_read_socket_thread(server_socket);

    while (1)
    {
        puts("You: ");
        clear_memory(buffer, sizeof (buffer));
        fgets(buffer, 255, stdin);

        if (write(server_socket, buffer, strlen(buffer)) < 0)
            error("ERROR writing to socket");


        if (strncmp(buffer, "exit\n", 5) == 0)
        {
            printf("CLOSING SOCKET FROM CLINET connection_host : %d\n", server_socket);
            shutdown(server_socket, SHUT_RDWR);
            break;
        }

    }
}
