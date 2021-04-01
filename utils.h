/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   utils.h
 * Author: piotr
 *
 * Created on June 5, 2019, 9:34 AM
 */

#ifndef UTILS_H
#define UTILS_H
#include <sys/select.h>

#ifdef __cplusplus
extern "C"
{
#endif

    void clear_memory(void *pointer, size_t size);
    void debug_hostent(struct hostent *connection_host);
    void debug_sockaddr_in(struct sockaddr_in *connection_host);
    int socket_can_read(fd_set mask, int sockfd, struct timeval *time_set);
    int socket_can_write(fd_set mask, int sockfd, struct timeval *time_set);

#ifdef __cplusplus
}
#endif

#endif /* UTILS_H */
