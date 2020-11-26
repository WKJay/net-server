/*************************************************
 Copyright (c) 2020
 All rights reserved.
 File name:     netserver.c
 Description:   
 History:
 1. Version:    
    Date:       2020-11-26
    Author:     wangjunjie
    Modify:     
*************************************************/
#include "netserver.h"
#include "ns_session.h"
#include "ns_platform.h"

#define NS_IF_LISTEN_BACKLOG 5

/**
 * Name:    netserver_create
 * Brief:   create netserber manager
 * Input:   
 *  @max_conns: max available connections
 *  @flag: server flag 
 *         NS_USE_TLS : use tls connection
 * Output:  netserver manager handler , NULL if create failed
 */
netserver_mgr_t *netserver_create(uint32_t max_conns, uint32_t flag) {
    netserver_mgr_t *mgr = NS_CALLOC(1, sizeof(netserver_mgr_t));
    if (mgr == NULL) {
        NS_LOG("no memory for netserver manager");
        return NULL;
    }
    if (flag & NS_USE_TLS) {
#if NS_SUPPORT_TLS
        mgr->flag |= NS_USE_TLS;
#else
        NS_LOG("TLS SUPPORT NOT AVAILABLE");
#endif
    }
    return mgr;
}

/**
 * Name:    netserver_bind
 * Brief:   bind port to netserver
 * Input:   
 *  @mgr:
 *  @port:
 * Output:  success:0
 */
int netserver_bind(netserver_mgr_t *mgr, uint16_t port) {
    ns_session_t *listen_session = NULL;
    if (mgr->listener) {
        NS_LOG("already have a listener");
        return -1;
    }

    listen_session = ns_session_create(NS_SESSION_F_LISTENING);
    if (listen_session == NULL) {
        NS_LOG("cannot create netserver session");
        return -1;
    }

    listen_session->socket = ns_if_socket();
    if (listen_session->socket < 0) {
        NS_LOG("create socket failed");
        return -1;
    }

    if (ns_if_bind(listen_session->socket, port) < 0) {
        NS_LOG("bind socket failed");
        ns_if_socket_close(listen_session->socket);
        return -1;
    }

    // if (ns_if_listen(listen_session->socket, NS_IF_LISTEN_BACKLOG) < 0) {
    //     NS_LOG("listen socket failed");
    //     ns_if_socket_close(listen_session->socket);
    //     return -1;
    // }

    return 0;
}

int netserver_start(netserver_mgr_t *mgr) {
    
}
