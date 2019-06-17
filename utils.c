#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h> 
#include <arpa/inet.h> 

void clear_memory(void * pointer, size_t size)
{
    memset(pointer, '\0', size);
}

void debug_hostent(struct hostent *connection_host)
{
    // char * SERWER_IP = inet_ntoa( **( struct in_addr ** ) connection_host->h_addr_list ) TO CHAR * IP
    // inet_pton( AF_INET, SERWER_IP, & serwer.sin_addr ) FROM CHAR * TO BigEndian unint32_t 

    printf("name: %s, h_addr_list: %s, lenght: %d\n",
            connection_host->h_name,
            inet_ntoa(**(struct in_addr **) connection_host->h_addr_list),
            connection_host->h_length);
}

void debug_sockaddr_in(struct sockaddr_in *connection_host)
{
    printf("sin_addr: %s, sin_family: %d, sin_port: %d\n",
            inet_ntoa(connection_host->sin_addr),
            connection_host->sin_family,
            connection_host->sin_port);
}