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
#include <rtthread.h>

#include "netserver.h"
#include "ns_session.h"
#include <sys/time.h>

#ifdef SAL_USING_POSIX
#include <sys/select.h>
#else
#include <lwip/select.h>
#endif

#define NS_IF_LISTEN_BACKLOG 6
#define NS_SELECT_TIMEOUT    2000

#define NS_THREAD_STACK_SIZE 8192
#define NS_THREAD_PRIORITY   10
#define NS_THREAD_TICK       5

#define NS_IS_RESET(s) (s & NS_RESET_FLAG)
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
    mgr->max_conns = max_conns;
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

static void check_session_timeout(netserver_mgr_t *mgr) {
    ns_session_t *conn = NULL;
    if (mgr->session_timeout == 0) {
        return;
    }
    for (conn = mgr->conns; conn; conn = conn->next) {
        if ((rt_tick_get() - conn->tick_timeout) < (RT_TICK_MAX / 2)) {
            NS_LOG("connecion %d timout,close it.", conn->socket);
            ns_session_close(mgr, conn);
        }
    }
}

static void netserver_accept_and_close(netserver_mgr_t *mgr) {
    int sock;
    struct sockaddr cliaddr;
    socklen_t clilen;
    clilen = sizeof(struct sockaddr_in);
    sock = accept(mgr->listener->socket, &cliaddr, &clilen);
    if (sock >= 0) {
        closesocket(sock);
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

void netserver_set_session_timeout(netserver_mgr_t *mgr, uint32_t ms) {
    mgr->session_timeout = ms;
}

static void netserver_handle(void *param) {
    netserver_mgr_t *mgr = (netserver_mgr_t *)param;
    struct sockaddr_in servaddr;
    fd_set readset, exceptfds, tempreadfds, tempexptfds;
    int maxfd = 0, sockfd = -1;
    struct timeval timeout;

    mgr->listener = ns_session_create(mgr, NS_SESSION_F_LISTENING);
    if (mgr->listener == NULL) {
        NS_LOG("cannot create netserver session");
        goto exit;
    }

    mgr->listener->socket = socket(AF_INET, SOCK_STREAM, 0);
    if (mgr->listener->socket < 0) {
        NS_LOG("create socket failed");
        goto exit;
    }

    NS_MEMSET(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(mgr->listen_port);
    if (bind(mgr->listener->socket, (struct sockaddr *)&servaddr,
             sizeof(servaddr)) < 0) {
        NS_LOG("bind socket failed");
        closesocket(mgr->listener->socket);
        goto exit;
    }

    if (listen(mgr->listener->socket, NS_IF_LISTEN_BACKLOG) < 0) {
        NS_LOG("listen socket failed");
        closesocket(mgr->listener->socket);
        goto exit;
    }

    timeout.tv_sec = NS_SELECT_TIMEOUT / 1000;
    timeout.tv_usec = (NS_SELECT_TIMEOUT % 1000) * 1000;
    /* waiting for new connection or data come in */
    for (;;) {
        FD_ZERO(&readset);
        FD_ZERO(&exceptfds);
        FD_SET(mgr->listener->socket, &readset);
        FD_SET(mgr->listener->socket, &exceptfds);

        maxfd = ns_all_connections_set_fds(mgr, &readset, &exceptfds);
        if (maxfd < mgr->listener->socket + 1)
            maxfd = mgr->listener->socket + 1;

        // prevent select from changing
        tempreadfds = readset;
        tempexptfds = exceptfds;

        sockfd = select(maxfd, &tempreadfds, NULL, &tempexptfds, &timeout);
        if (NS_IS_RESET(mgr->flag)) {
            NS_LOG("net server reseting...");
            goto exit;
        }
        check_session_timeout(mgr);
        if (sockfd == 0) {
            // NS_LOG("net server select timeout");
            continue;
        }

        /* if the listen fd is ready*/
        if (FD_ISSET(mgr->listener->socket, &tempreadfds)) {
            socklen_t clilen;
            NS_LOG("new connection comes in");
            clilen = sizeof(struct sockaddr_in);

            ns_session_t *new_conn = ns_session_create(mgr, NULL);
            if (new_conn) {
                new_conn->socket =
                    accept(mgr->listener->socket,
                           (struct sockaddr *)&new_conn->cliaddr, &clilen);
                if (new_conn->socket < 0) {
                    NS_LOG("new connection accept failed");
                    ns_session_close(mgr, new_conn);
                } else {
                    FD_SET(new_conn->socket, &readset);
                    FD_SET(new_conn->socket, &exceptfds);
                }
            } else {
                /* cannot create new connection,just accept and close it */
                NS_LOG("accept connection and close");
                netserver_accept_and_close(mgr);
            }
        }

        /* handle sessions */
        ns_session_handle(mgr, &tempreadfds, &tempexptfds);
    }

exit:
    netserver_close_all(mgr);
}

void netserver(void *param) {
    while (1) {
        netserver_handle(param);
        rt_thread_mdelay(1000);
    }
}

int netserver_start(netserver_mgr_t *mgr) {
    rt_thread_t tid =
        rt_thread_create("netserver", netserver, mgr, NS_THREAD_STACK_SIZE,
                         NS_THREAD_PRIORITY, NS_THREAD_TICK);
    if (tid) {
        if (rt_thread_startup(tid) == RT_EOK) {
            return 0;
        } else {
            return -1;
        }
    } else {
        return -1;
    }
}
