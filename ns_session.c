/*************************************************
 Copyright (c) 2020
 All rights reserved.
 File name:     ns_session.c
 Description:
 History:
 1. Version:
    Date:       2020-11-26
    Author:     wangjunjie
    Modify:
*************************************************/
#include "netserver.h"
#include "ns_session.h"
#include "ns_if_socket.h"


#define IS_LISTEN_SESSION(s) (s->flag & NS_SESSION_F_LISTENING)

/**
 * Name:    ns_session_create
 * Brief:   create netserber session
 * Input:
 *  @flag: session flag
 *         NS_USE_TLS : use tls connection
 * Output:  netserver manager handler , NULL if create failed
 */
ns_session_t *ns_session_create(uint32_t flag) {
    ns_session_t *session = NS_CALLOC(1, sizeof(ns_session_t));
    if (session == NULL) {
        NS_LOG("no memory for ns session");
        return NULL;
    }
    session->flag = flag;
    return session;
}

int ns_session_close(netserver_mgr_t *mgr, ns_session_t *session) {
    ns_session_t *iter;

    if (IS_LISTEN_SESSION(session)) {
        /* listen session free */
        if (mgr->listener != session) {
            NS_LOG("listener error");
            return -1;
        }
        // TODO maybe you need to process close failure
        ns_if_socket_close(session->socket);
#if NS_SUPPORT_TLS
        ns_tls_free(session->tls_backend);
        session->tls_backend = NULL;
#endif
        NS_FREE(session);
        mgr->listener = NULL;
    } else {
        /* client session free */
        if (mgr->conns == session) {
            mgr->conns = session->next;
        } else {
            for (iter = mgr->conns; iter; iter = iter->next) {
                if (iter->next == session) {
                    iter->next = session->next;
                    break;
                }
            }
        }
        NS_FREE(session);
    }
    return 0;
}

/**
 * Name:    ns_sesion_free_all_connections
 * Brief:   This function will free all connections
 *          except listen session
 * Input:   None
 * Output:  success:0
 */
int ns_sesion_close_all_connections(netserver_mgr_t *mgr) {
    ns_session_t *cur_conn, *next_conn;
    for (cur_conn = mgr->conns; cur_conn; cur_conn = next_conn) {
        /* obtain next connection in case it be closed */
        next_conn = cur_conn->next;
        if (ns_session_close(mgr,cur_conn) < 0) {
            NS_LOG("close session failed");
            return -1;
        }
    }
}
