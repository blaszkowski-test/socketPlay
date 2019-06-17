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

#ifdef __cplusplus
extern "C"
{
#endif

    void server(unsigned short port_number);
    void *one_client(void *arg);
    void send_to_all_clients(char * message, long client_id);
    void remove_one_client(long client_id);
    int bind_socket(unsigned short port_number);

#ifdef __cplusplus
}
#endif

#endif /* SERVER_H */

