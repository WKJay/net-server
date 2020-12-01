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
#if NS_ENABLE_SSL
#include "ns_ssl_if.h"
#endif

#define IS_LISTEN_SESSION(s) (s->flag & NS_SESSION_F_LISTENING)

static ns_session_t *find_last_connection(netserver_mgr_t *mgr, int *cnt) {
    ns_session_t *conn = mgr->conns;
    if (mgr->conns == NULL) {
        /* There's no connection */
        return NULL;
    }
    (*cnt)++;
    while (conn->next) {
        conn = conn->next;
        (*cnt)++;
    }
    return conn;
}

static void _session_handle(netserver_mgr_t *mgr, ns_session_t *conn) {
    int ret = 0;
    uint8_t buf[256];
    NS_MEMSET(buf, 0, 256);

    conn->tick_timeout =
        rt_tick_get() + rt_tick_from_millisecond(mgr->session_timeout);

    ret = recv(conn->socket, buf, 256, 0);
    if (ret > 0) {
        send(conn->socket, buf, ret, 0);
    } else {
        NS_LOG("socket %d read err,close it", conn->socket);
        ns_session_close(mgr, conn);
    }
}

/**
 * Name:    ns_session_create
 * Brief:   create netserber session
 * Input:
 *  @flag: session flag
 *         NS_USE_TLS : use tls connection
 * Output:  netserver manager handler , NULL if create failed
 */
ns_session_t *ns_session_create(netserver_mgr_t *mgr, uint32_t flag) {
    ns_session_t *session = NS_CALLOC(1, sizeof(ns_session_t));
    if (session == NULL) {
        NS_LOG("no memory for ns session");
        return NULL;
    }

    if (flag & NS_SESSION_F_LISTENING) {
        if (mgr->listener) {
            NS_LOG("already have a listener");
            NS_FREE(session);
            return NULL;
        } else {
            mgr->listener = session;
        }
    } else {
        int conn_cnt = 0;
        ns_session_t *last_conn = find_last_connection(mgr, &conn_cnt);
        if (last_conn) {
            if (conn_cnt >= mgr->max_conns) {
                NS_LOG("no more connections");
                NS_FREE(session);
                return NULL;
            } else {
                last_conn->next = session;
            }
        } else {
            mgr->conns = session;
        }
    }

    session->flag = flag;
    session->socket = -1;
    session->tick_timeout =
        rt_tick_get() + rt_tick_from_millisecond(mgr->session_timeout);
    return session;
}

int ns_session_close(netserver_mgr_t *mgr, ns_session_t *session) {
    ns_session_t *iter;

    if (session->socket >= 0) closesocket(session->socket);

    if (IS_LISTEN_SESSION(session)) {
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
        if (ns_session_close(mgr, cur_conn) < 0) {
            NS_LOG("close session failed");
            return -1;
        }
    }
    return 0;
}

/**
 * Name:    ns_all_connections_set_fds
 * Brief:   set all connections fds
 * Input:   None
 * Output:  maximal fd number
 */
int ns_all_connections_set_fds(netserver_mgr_t *mgr, fd_set *readset,
                               fd_set *exceptfds) {
    int maxfdp1 = 0;
    ns_session_t *session;

    for (session = mgr->conns; session; session = session->next) {
        if (maxfdp1 < session->socket + 1) maxfdp1 = session->socket + 1;
        FD_SET(session->socket, readset);
        FD_SET(session->socket, exceptfds);
    }
    return maxfdp1;
}

void ns_session_handle(netserver_mgr_t *mgr, fd_set *readset,
                       fd_set *excptset) {
    ns_session_t *cur_conn, *next_conn;

    for (cur_conn = mgr->conns; cur_conn; cur_conn = next_conn) {
        // obtain next session in case current session be closed
        next_conn = cur_conn->next;
        if (FD_ISSET(cur_conn->socket, excptset)) {
            NS_LOG("socket %d error,now close", cur_conn->socket);
            ns_session_close(mgr, cur_conn);
        } else if (FD_ISSET(cur_conn->socket, readset)) {
            _session_handle(mgr, cur_conn);
        }
    }
}
