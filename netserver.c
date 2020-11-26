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

static void netserver_close_all(netserver_mgr_t *mgr) {
    /* close and free all connections */
    if (mgr->conns) {
        ns_sesion_close_all_connections(mgr);
        mgr->conns = NULL;
    }
    /* close listen session*/
    if (mgr->listener) {
        ns_session_close(mgr, mgr->listener);
        mgr->listener = NULL;
    }
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
    mgr->listen_port = port;
    return 0;
}

void netserver(netserver_mgr_t *mgr) {
    if (mgr->listener) {
        NS_LOG("already have a listener");
        goto exit;
    }

    mgr->listener = ns_session_create(NS_SESSION_F_LISTENING);
    if (mgr->listener == NULL) {
        NS_LOG("cannot create netserver session");
        goto exit;
    }

    mgr->listener->socket = ns_if_socket();
    if (mgr->listener->socket < 0) {
        NS_LOG("create socket failed");
        goto exit;
    }

    if (ns_if_bind(mgr->listener->socket, mgr->listen_port) < 0) {
        NS_LOG("bind socket failed");
        ns_if_socket_close(mgr->listener->socket);
        goto exit;
    }

    if (ns_if_listen(mgr->listener->socket, NS_IF_LISTEN_BACKLOG) < 0) {
        NS_LOG("listen socket failed");
        ns_if_socket_close(mgr->listener->socket);
        goto exit;
    }

    /* waiting for new connection or data come in */
    for(;;)
    {
        
    }

exit:
    netserver_close_all(mgr);
}

int netserver_start(netserver_mgr_t *mgr) { ns_thread_start(netserver, mgr); }
