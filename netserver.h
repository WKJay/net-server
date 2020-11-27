#ifndef __NETSERVER_H
#define __NETSERVER_H

#include "ns_types.h"

#define NS_USE_TLS       (1 << 0)
#define NS_RESET_FLAG    (1 << 1)
#define NS_SHUTDOWN_FLAG (1 << 2)

/**
 * netserver manager
 */
typedef struct _netserver_mgr {
    struct _ns_session *listener;  // listen session
    struct _ns_session *conns;     // session list
    uint16_t listen_port;          // server listen port
    uint32_t max_conns;            // max connections
    uint32_t session_timeout;      // session timeout
    uint32_t flag;                 // status flag
} netserver_mgr_t;

/**
 * API definition
 */
netserver_mgr_t *netserver_create(uint32_t max_conns, uint32_t flag);
int netserver_bind(netserver_mgr_t *mgr, uint16_t port);
int netserver_start(netserver_mgr_t *mgr);
void netserver_set_session_timeout(netserver_mgr_t *mgr, uint32_t ms);

#endif /* __NETSERVER_H */
