#ifndef __NETSERVER_H
#define __NETSERVER_H

#include "ns_session.h"

#define NS_USE_TLS       (1 << 0)
#define NS_RESET_FLAG    (1 << 1)
#define NS_SHUTDOWN_FLAG (1 << 2)

/**
 * netserver manager
 */
typedef struct _netserver_mgr {
    ns_session_t *listener;  // listen session
    ns_session_t *conn;      //session list
    uint32_t max_conns;      //max connections
    uint32_t flag;           //status flag
} netserver_mgr_t;

/**
 * API definition
 */
netserver_mgr_t *netserver_create(uint32_t max_conns, uint32_t flag);

#endif /* __NETSERVER_H */
