/*************************************************
 Copyright (c) 2020
 All rights reserved.
 File name:     ns_ssl_if.c
 Description:
 History:
 1. Version:
    Date:       2020-11-30
    Author:     WKJay
    Modify:
*************************************************/
#include "user_settings.h"
#include <wolfssl/ssl.h>
#include <wolfssl/internal.h>

#include "netserver.h"

#define CHECK_IF_MEMORY_ALLOCATED(ptr, name, ret) \
    if (ptr == NULL) {                            \
        NS_LOG("no memory for %s.", name);        \
        return ret;                               \
    }

typedef struct _wolfssl_backend {
    WOLFSSL_CTX *ctx;  // only for listen session
    WOLFSSL *ssl;      // for client connections
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

    /* free backend */
    NS_FREE(backend);
}

/**
 * Name:    ns_ssl_if_context_create
 * Brief:   create ssl interface context
 * Input:
 * @mgr:        netserver manager
 * Output:  success:0,error:-1
 */
int ns_ssl_if_context_create(netserver_mgr_t *mgr) {
    WOLFSSL_METHOD *method = NULL;
    wolfssl_backend_t *backend = NULL;
    netserver_opt_t *opts = &mgr->opts;
    int mode = 0;

    /* Create wolfssl backend struct */
    backend = NS_CALLOC(1, sizeof(wolfssl_backend_t));
    CHECK_IF_MEMORY_ALLOCATED(backend, "wolfssl backend", -1);

    /* Init wolfSSL library */
    wolfSSL_Init();

    // wolfSSL_Debugging_ON();

    /* Create wolfSSL context */
    method = wolfTLSv1_2_server_method();
    backend->ctx = wolfSSL_CTX_new(method);
    if (backend->ctx == NULL) {
        NS_LOG("wolfSSL context create failed.");
        NS_FREE(backend);
        return -1;
    }

    /* Load private key and certificate */
    if (opts->server_cert_buffer == NULL || opts->server_key_buffer == NULL) {
#ifdef NO_FILESYSTEM
        NS_LOG("no private key or certificate provided,please check!");
        return -1;
#else
        goto load_cert_file;
#endif
    }

    if (wolfSSL_CTX_use_PrivateKey_buffer(
            backend->ctx, (const unsigned char *)opts->server_key_buffer,
            strlen(opts->server_key_buffer), WOLFSSL_FILETYPE_PEM) != SSL_SUCCESS) {
        NS_LOG("load private key buffer failed.");
        goto exit;
    }

    if (wolfSSL_CTX_use_certificate_buffer(
            backend->ctx, (const unsigned char *)opts->server_cert_buffer,
            strlen(opts->server_cert_buffer), WOLFSSL_FILETYPE_PEM) != SSL_SUCCESS) {
        NS_LOG("load certificate buffer failed.");
        goto exit;
    }

    if (opts->ca_cert_buffer) {
        if (wolfSSL_CTX_load_verify_buffer_ex(backend->ctx,
                                              (const unsigned char *)opts->ca_cert_buffer,
                                              strlen(opts->ca_cert_buffer), WOLFSSL_FILETYPE_PEM, 0,
                                              WOLFSSL_LOAD_FLAG_DATE_ERR_OKAY) != SSL_SUCCESS) {
            NS_LOG("load ca certificate buffer failed.");
            goto exit;
        }
    }

#ifndef NO_FILESYSTEM
load_cert_file:
    if (opts->server_cert == NULL || opts->server_key == NULL) {
        NS_LOG("private key or certificate path error,please check!");
        return -1;
    }
    if (wolfSSL_CTX_use_PrivateKey_file(backend->ctx, opts->server_key, SSL_FILETYPE_PEM) !=
        SSL_SUCCESS) {
        NS_LOG("load private key %s failed.", opts->server_key);
        goto exit;
    }
    if (wolfSSL_CTX_use_certificate_file(backend->ctx, opts->server_cert, SSL_FILETYPE_PEM) !=
        SSL_SUCCESS) {
        NS_LOG("load certificate %s failed.", opts->server_cert);
        goto exit;
    }
    if (opts->ca_cert) {
        if (wolfSSL_CTX_load_verify_locations_ex(backend->ctx, opts->ca_cert, NULL,
                                                 WOLFSSL_LOAD_FLAG_DATE_ERR_OKAY) != SSL_SUCCESS) {
            NS_LOG("load ca %s failed.", opts->ca_cert);
            goto exit;
        }
    }
#endif

    /* set verify mode */
    if (mgr->flag & NS_SSL_VERIFY_PEER) mode |= SSL_VERIFY_PEER;
    if (mgr->flag & NS_SSL_FORCE_PEER_CERT) mode |= SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
    if (mode) wolfSSL_CTX_set_verify(backend->ctx, mode, NULL);

    mgr->listener->ssl_if_data = backend;
    return 0;
exit:
    if (backend) wolfssl_backend_free(backend);
    wolfSSL_Cleanup();
    return -1;
}

int ns_ssl_if_handshake(netserver_mgr_t *mgr, ns_session_t *session) {
    wolfssl_backend_t *backend = NULL;
    wolfssl_backend_t *ls_back = (wolfssl_backend_t *)mgr->listener->ssl_if_data;

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
    /* notify user */
    if (mgr->opts.callback.ssl_handshake_cb) {
#if defined(KEEP_PEER_CERT)
        if (backend->ssl->peerCert.derCert) {
            DerBuffer *peerCert = backend->ssl->peerCert.derCert;
            mgr->opts.callback.ssl_handshake_cb(session, peerCert->buffer, peerCert->length);
        } else {
            mgr->opts.callback.ssl_handshake_cb(session, NULL, 0);
        }
#else
        mgr->opts.callback.ssl_handshake_cb(session, NULL, 0);
#endif
    }
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
    WOLFSSL *ssl = ((wolfssl_backend_t *)session->ssl_if_data)->ssl;
    return wolfSSL_read(ssl, data, sz);
}

int ns_ssl_if_write(ns_session_t *session, void *data, int sz) {
    if (session->ssl_if_data == NULL) {
        NS_LOG("invalid ssl interface data.");
    }
    WOLFSSL *ssl = ((wolfssl_backend_t *)session->ssl_if_data)->ssl;
    return wolfSSL_write(ssl, data, sz);
}
