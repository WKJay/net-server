/*************************************************
 Copyright (c) 2020
 All rights reserved.
 File name:     simple_ssl_server.c
 Description:
 History:
 1. Version:
    Date:       2020-12-06
    Author:     WKJay
    Modify:
*************************************************/
#include <stdio.h>
#include "netserver.h"

static int netserver_readable_cb(ns_session_t *ns, void *data, int sz) {
    int ret = 0;
    ret = netserver_write(ns, data, sz);
    return ret;
}

int ssl_server_init(void) {
    netserver_opt_t opts;
    netserver_mgr_t *mgr = NULL;
    rt_memset(&opts, 0, sizeof(opts));

    opts.max_conns = 3;
    opts.listen_port = 3334;

    /* disconnect connection after one minute no data input */
    opts.session_timeout = 60 * 1000;

    /* default stack size may not be enough */
    opts.thread_attrs.stack_size = 6 * 1024;

    /* load certificates */
    opts.server_cert = "/sdcard/test/server_cert.pem";
    opts.server_key = "/sdcard/test/private_key.pem";
    /* maybe needed if you want to verify peer */
    opts.ca_cert = "/sdcard/test/ca_cert.pem";

    /* register callback function */
    opts.callback.data_readable_cb = netserver_readable_cb;

    /* create netserver manager object */
    mgr = netserver_create(&opts, NS_USE_SSL);
    if (mgr == NULL) {
        printf("create simple ssl server manager failed.\r\n");
        return -1;
    }

    /* start netserver */
    if (netserver_start(mgr) == 0) {
        printf("start simple ssl server on port %d.\r\n", opts.listen_port);
        return 0;
    } else {
        printf("start simple ssl server error.\r\n");
        return -1;
    }
}
