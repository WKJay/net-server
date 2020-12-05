/*************************************************
 Copyright (c) 2020
 All rights reserved.
 File name:     simple_tcp_server.c
 Description:
 History:
 1. Version:
    Date:       2020-12-05
    Author:     wangjunjie
    Modify:
*************************************************/
#include <stdio.h>
#include "netserver.h"

netserver_mgr_t *mgr = NULL;

static int netserver_readable_cb(ns_session_t *ns, void *data, int sz) {
    int ret = 0;
    ret = netserver_write(ns, data, sz);
    return ret;
}

int tcp_server_init(void) {
    netserver_opt_t opts;
    rt_memset(&opts, 0, sizeof(opts));
    opts.max_conns = 3;
    opts.listen_port = 3333;
    /* disconnect connection after one minute no data input */
    opts.session_timeout = 1 * 60 * 1000;

    /* register callback function*/
    opts.callback.data_readable_cb = netserver_readable_cb;

    /* create netserver manager object */
    mgr = netserver_create(&opts, NULL);
    if (mgr == NULL) {
        printf("create netserver manager failed.\r\n");
        return -1;
    }

    /* start netserver */
    if (netserver_start(mgr) == 0) {
        printf("start netserver on port %d.\r\n", opts.listen_port);
        return 0;
    } else {
        printf("start netserver error.\r\n");
        return -1;
    }
}
