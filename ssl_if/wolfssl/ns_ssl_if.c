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

typedef struct _wolfssl_backend {
    WOLFSSL *ssl;
    void *user_data;
} wolfssl_backend_t;

/**
 * Name:    ns_ssl_if_context_create
 * Brief:   create ssl interface context
 * Input:
 * @mgr:        netserver manager
 * @opts:       SSL options
 * Output:  success:0,error:-1
 */
int ns_ssl_if_context_create(netserver_mgr_t *mgr, netserver_opt_t *opts) {
    WOLFSSL_CTX *ctx = NULL;
    WOLFSSL_METHOD *method = NULL;

    /* Init wolfSSL library */
    wolfSSL_Init();

    /* Create wolfSSL context */
    method = wolfTLSv1_2_server_method();
    ctx = wolfSSL_CTX_new(method);
    if (ctx == NULL) {
        NS_LOG("wolfSSL context create failed.");
        return NULL;
    }

    /* Load private key and certificate */
    if (opts->server_cert == NULL || opts->server_key == NULL) {
        NS_LOG("private key or certificate path error,please check!");
        return NULL;
    }
    if (wolfSSL_CTX_use_PrivateKey_file(ctx, opts->server_key,
                                        SSL_FILETYPE_PEM) != SSL_SUCCESS) {
        NS_LOG("load private key %s failed.", opts->server_key);
        goto exit;
    }
    if (wolfSSL_CTX_use_certificate_file(ctx, opts->server_cert,
                                         SSL_FILETYPE_PEM) != SSL_SUCCESS) {
        NS_LOG("load certificate %s failed.", opts->server_cert);
        goto exit;
    }

    mgr->listener->ssl_if_data = ctx;
    return 0;
exit:
    if (ctx) wolfSSL_CTX_free(ctx);
    wolfSSL_Cleanup();
    return -1;
}

int ns_ssl_if_handshake(netserver_mgr_t *mgr, ns_session_t *conn) {
    WOLFSSL *ssl = NULL;
    ssl = wolfSSL_new((WOLFSSL_CTX *)mgr->listener->ssl_if_data);
    if (ssl == NULL) {
        NS_LOG("SSL session create failed.");
        return -1;
    }
    
}
