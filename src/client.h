/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   client.h
 * Author: piotr
 *
 * Created on June 5, 2019, 8:38 AM
 */

#ifndef CLIENT_H
#define CLIENT_H
#include <netdb.h> 

#ifdef __cplusplus
extern "C"
{
#endif

    void * client_screen_tcp(void * arg);
    void * client_screen_udp(void * arg);
    void client(const char *type, const char* host_name, unsigned short port_number);
    long connect_to_server_tcp(const char* host_name, unsigned short port_number);
    long connect_to_server_udp(const char* host_name, unsigned short port_number);
    void create_read_socket_thread(long server_socket, const char *type);
    void setup_server_settings(struct hostent *connection_host, unsigned short port_number);

#ifdef __cplusplus
}
#endif

#endif /* CLIENT_H */

