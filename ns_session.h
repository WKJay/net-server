#ifndef __NS_SESSION_H
#define __NS_SESSION_H

#include <rtthread.h>
#include "netserver.h"
#include "ns_types.h"

#ifdef RT_USING_SAL
#include <sys/socket.h>
#else
#include <lwip/sockets.h>
#endif

/* Flags set by net server*/
#define NS_SESSION_F_LISTENING (1 << 0)
#define NS_SESSION_F_SSL       (1 << 1)

typedef struct _ns_session {
    int socket;
    struct sockaddr_in cliaddr;
    uint32_t tick_timeout;
    uint32_t flag;
    struct _ns_session *next;
    void *user_data;
#if NS_ENABLE_SSL
    void *ssl_if_data;
#else
    void *unused_data; /* To keep the size of the structure the same */
#endif
} ns_session_t;

ns_session_t *ns_session_create(netserver_mgr_t *mgr, uint32_t flag);
int ns_session_close(netserver_mgr_t *mgr, ns_session_t *session);
int ns_sesion_close_all_connections(netserver_mgr_t *mgr);
int ns_all_connections_set_fds(netserver_mgr_t *mgr, fd_set *readset,
                               fd_set *exceptfds);
void ns_session_handle(netserver_mgr_t *mgr, fd_set *readset, fd_set *excptset);
int netserver_read(ns_session_t *ns, void *data, int sz);
int netserver_write(ns_session_t *ns, void *data, int sz);
#endif /* __NS_SESSION_H */
