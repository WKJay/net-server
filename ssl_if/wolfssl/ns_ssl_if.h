#ifndef __NS_SSL_IF_H
#define __NS_SSL_IF_H

#include "netserver.h"
#include "ns_session.h"

int ns_ssl_if_context_create(netserver_mgr_t *mgr, netserver_opt_t *opts);
int ns_ssl_if_handshake(netserver_mgr_t *mgr, ns_session_t *conn);
void ns_ssl_if_free(ns_session_t *session);
int ns_ssl_if_read(ns_session_t *session, void *data, int sz);
int ns_ssl_if_write(ns_session_t *session, void *data, int sz);

#endif /* __NS_SSL_IF_H */
