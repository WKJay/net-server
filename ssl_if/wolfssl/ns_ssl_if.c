/*************************************************
 Copyright (c) 2020
 All rights reserved.
 File name:     ns_ssl_if.c
 Description:
 History:
 1. Version:
    Date:       2020-11-30
    Author:     wangjunjie
    Modify:
*************************************************/
#include <wolfssl/options.h>
#include <wolfssl/ssl.h>

#include "netserver.h"
#include "ns_session.h"

#define CHECK_IF_MEMORY_ALLOCATED(ptr, name, ret) \
    if (ptr == NULL) {                            \
        NS_LOG("no memory for %s.", name);        \
        return ret;                               \
    }

typedef struct _wolfssl_backend {
    WOLFSSL_CTX *ctx;  // only for listen session
    WOLFSSL *ssl;      // for client connections
    // void *user_data;   // user specific data
} wolfssl_backend_t;

static void wolfssl_backend_free(wolfssl_backend_t *backend) {
    /* free wolfssl context */
    if (backend->ctx) {
        wolfSSL_CTX_free(backend->ctx);
        backend->ctx = NULL;
    }

    /* free ssl session*/
    if (backend->ssl) {
        wolfSSL_free(backend->ssl);
        backend->ssl = NULL;
    }

    /* call user function to free user data */

    /* free backend */
    NS_FREE(backend);
}

/**
 * Name:    ns_ssl_if_context_create
 * Brief:   create ssl interface context
 * Input:
 * @mgr:        netserver manager
 * @opts:       SSL options
 * Output:  success:0,error:-1
 */
int ns_ssl_if_context_create(netserver_mgr_t *mgr, netserver_opt_t *opts) {
    WOLFSSL_METHOD *method = NULL;
    wolfssl_backend_t *backend = NULL;

    /* Create wolfssl backend struct */
    backend = NS_CALLOC(1, sizeof(wolfssl_backend_t));
    CHECK_IF_MEMORY_ALLOCATED(backend, "wolfssl backend", -1);

    /* Init wolfSSL library */
    wolfSSL_Init();

    /* Create wolfSSL context */
    method = wolfTLSv1_2_server_method();
    backend->ctx = wolfSSL_CTX_new(method);
    if (backend->ctx == NULL) {
        NS_LOG("wolfSSL context create failed.");
        NS_FREE(backend);
        return -1;
    }

    /* Load private key and certificate */
    if (opts->server_cert == NULL || opts->server_key == NULL) {
        NS_LOG("private key or certificate path error,please check!");
        return NULL;
    }
    if (wolfSSL_CTX_use_PrivateKey_file(backend->ctx, opts->server_key,
                                        SSL_FILETYPE_PEM) != SSL_SUCCESS) {
        NS_LOG("load private key %s failed.", opts->server_key);
        goto exit;
    }
    if (wolfSSL_CTX_use_certificate_file(backend->ctx, opts->server_cert,
                                         SSL_FILETYPE_PEM) != SSL_SUCCESS) {
        NS_LOG("load certificate %s failed.", opts->server_cert);
        goto exit;
    }

    mgr->listener->ssl_if_data = backend;
    return 0;
exit:
    if (backend) wolfssl_backend_free(backend);
    wolfSSL_Cleanup();
    return -1;
}

int ns_ssl_if_handshake(netserver_mgr_t *mgr, ns_session_t *session) {
    wolfssl_backend_t *backend = NULL;
    wolfssl_backend_t *ls_back =
        (wolfssl_backend_t *)mgr->listener->ssl_if_data;

    /* Create wolfssl backend struct */
    backend = NS_CALLOC(1, sizeof(wolfssl_backend_t));
    CHECK_IF_MEMORY_ALLOCATED(backend, "wolfssl backend", -1);

    /* Create tls session */
    backend->ssl = wolfSSL_new(ls_back->ctx);
    if (backend->ssl == NULL) {
        NS_LOG("SSL session create failed.");
        goto exit;
    }

    /* Set fd */
    wolfSSL_set_fd(backend->ssl, session->socket);

    /* do handshake */
    do {
        int ret = wolfSSL_accept(backend->ssl);
        if (ret != SSL_SUCCESS) {
            NS_LOG("SSL handshake failed.");
            goto exit;
        }
    } while (wolfSSL_want_read(backend->ssl));
    session->ssl_if_data = backend;
    return 0;

exit:
    if (backend) wolfssl_backend_free(backend);
    return -1;
}

void ns_ssl_if_free(ns_session_t *session) {
    if (session->ssl_if_data) {
        wolfssl_backend_free(session->ssl_if_data);
        session->ssl_if_data = NULL;
    }
}

int ns_ssl_if_read(ns_session_t *session, void *data, int sz) {
    if (session->ssl_if_data == NULL) {
        NS_LOG("invalid ssl interface data.");
    }
    WOLFSSL *ssl = (WOLFSSL *)session->ssl_if_data;
    return wolfSSL_read(ssl, data, sz);
}

int ns_ssl_if_write(ns_session_t *session, void *data, int sz) {
    if (session->ssl_if_data == NULL) {
        NS_LOG("invalid ssl interface data.");
    }
    WOLFSSL *ssl = (WOLFSSL *)session->ssl_if_data;
    return wolfSSL_write(ssl, data, sz);
}
