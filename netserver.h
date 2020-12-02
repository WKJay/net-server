#ifndef __NETSERVER_H
#define __NETSERVER_H

#include "ns_types.h"
#ifdef RT_USING_SAL
#include <sys/socket.h>
#else
#include <lwip/sockets.h>
#endif

#define NS_USE_SSL       (1 << 0)
#define NS_RESET_FLAG    (1 << 1)
#define NS_SHUTDOWN_FLAG (1 << 2)

/**
 * netserver session struct
 * */
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

/**
 * netserver callback
 * */
typedef struct _netserver_cb {
    void (*session_create_cb)(ns_session_t *session);
    void (*session_close_cb)(ns_session_t *session);
    int (*data_readable_cb)(ns_session_t *session, void *data, int sz);
#if NS_ENABLE_SSL
    int (*ssl_handshake_cb)(ns_session_t *session, void *cert_data,
                          int cert_size);
#endif
} netserver_cb_t;

/**
 * netserver options
 */
typedef struct _netserver_opt {
    uint16_t listen_port;        // server listen port
    uint32_t data_pkg_max_size;  // max size of data package
    uint32_t max_conns;          // max connections
    uint32_t session_timeout;    // session timeout
    netserver_cb_t callback;     // callback functions
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
    ns_session_t *listener;  // listen session
    ns_session_t *conns;     // session list
    uint8_t *data_buff;      // data buffer
    netserver_opt_t opts;   // options
    uint32_t flag;           // status flag
} netserver_mgr_t;

/**
 * API definition
 */
netserver_mgr_t *netserver_create(netserver_opt_t *opts, uint32_t flag);
int netserver_bind_options(netserver_mgr_t *mgr, netserver_opt_t *opts);
int netserver_start(netserver_mgr_t *mgr);
int netserver_mgr_free(netserver_mgr_t *mgr);
void netserver_set_session_timeout(netserver_mgr_t *mgr, uint32_t ms);
int netserver_read(ns_session_t *ns, void *data, int sz);
int netserver_write(ns_session_t *ns, void *data, int sz);

#endif /* __NETSERVER_H */
