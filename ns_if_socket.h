#ifndef __NS_SOCKET_H
#define __NS_SOCKET_H

#include <sys/socket.h>
#include "ns_types.h"

typedef struct sockaddr_in ns_sockaddr_in;
typedef int socket_t;

int ns_if_socket(void);
int ns_if_bind(socket_t socket, uint16_t port);
int ns_if_listen(socket_t socket, int backlog);
int ns_if_socket_close(socket_t socket);

#endif /* __NS_SOCKET_H */
