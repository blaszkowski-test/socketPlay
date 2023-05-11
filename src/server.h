/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   server.h
 * Author: piotr
 *
 * Created on June 5, 2019, 8:40 AM
 */

#ifndef SERVER_H
#define SERVER_H
#include <netinet/in.h>

#ifdef __cplusplus
extern "C"
{
#endif

    void server(unsigned short port_number, const char * type);
    void *one_client(void *arg);
    void send_to_all_clients_tcp(char * message, long client_id);
    void send_to_all_clients_udp(int sockfd, const char *message, struct sockaddr_in *client);
    void remove_one_client(long client_id);
    int bind_socket_tcp(unsigned short port_number);
    int bind_socket_udp(unsigned short port_number);
    void udp_push_list(struct sockaddr_in * client);

#ifdef __cplusplus
}
#endif

#endif /* SERVER_H */

