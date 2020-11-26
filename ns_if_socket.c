/*************************************************
 Copyright (c) 2020
 All rights reserved.
 File name:     ns_if_socket.c
 Description:   
 History:
 1. Version:    
    Date:       2020-11-26
    Author:     wangjunjie
    Modify:     
*************************************************/
#include "ns_if_socket.h"

int ns_if_socket(void) {
    int sock = 0, reuse = 1;
    sock = socket(AF_INET, SOCK_STREAM, 0);

    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    return sock;
}

int ns_if_bind(socket_t socket, uint16_t port) {
    struct sockaddr_in servaddr;
    NS_MEMSET(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    return bind(socket, &servaddr, sizeof(servaddr));
}

int ns_if_listen(socket_t socket, int backlog) {
    return listen(socket, backlog);
}

int ns_if_socket_close(socket_t socket) {
    return closesocket(socket);
}
