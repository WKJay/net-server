#ifndef __NS_SESSION_H
#define __NS_SESSION_H

#include "ns_types.h"
#include "ns_if_socket.h"

/* Flags set by net server*/
#define NS_SESSION_F_LISTENING (1 << 0)

typedef struct _ns_session {
    socket_t socket;
    ns_sockaddr_in cliaddr;
    uint32_t tick_timeout;
    uint32_t flag;
    struct _ns_session *next;
    void *user_data;
#if NS_SUPPORT_TLS
    void *tls_backend;
#else
    void *unused_data; /* To keep the size of the structure the same */
#endif
} ns_session_t;


ns_session_t *ns_session_create(uint32_t flag);

#endif /* __NS_SESSION_H */
