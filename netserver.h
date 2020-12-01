#ifndef __NETSERVER_H
#define __NETSERVER_H

#include "ns_types.h"

#define NS_USE_SSL       (1 << 0)
#define NS_RESET_FLAG    (1 << 1)
#define NS_SHUTDOWN_FLAG (1 << 2)

/**
 * netserver callback
 * */
typedef struct _netserver_cb {
    int (*data_readable_cb)(void *session);
    void (*session_close_cb)(void *session);
#if NS_ENABLE_SSL
    int (*peer_verify_cb)(void *cert_data, int cert_size);
#endif
} netserver_cb_t;

/**
 * netserver options
 */
typedef struct _netserver_opt {
    uint16_t listen_port;      // server listen port
    uint32_t max_conns;        // max connections
    uint32_t session_timeout;  // session timeout
    netserver_cb_t callback;  // callback functions
#if NS_ENABLE_SSL
    const char *server_key;
    const char *server_cert;
    const char *ca_cert;
#endif

} netserver_opt_t;

/**
 * netserver manager
 */
typedef struct _netserver_mgr {
    struct _ns_session *listener;  // listen session
    struct _ns_session *conns;     // session list
    netserver_opt_t *opts;         // options
    uint32_t flag;                 // status flag
} netserver_mgr_t;

/**
 * API definition
 */
netserver_mgr_t *netserver_create(netserver_opt_t *opts, uint32_t flag);
int netserver_bind_options(netserver_mgr_t *mgr, netserver_opt_t *opts);
int netserver_start(netserver_mgr_t *mgr);
void netserver_set_session_timeout(netserver_mgr_t *mgr, uint32_t ms);

#endif /* __NETSERVER_H */
