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
#include <rtthread.h>
#include <stdio.h>
#include "netserver.h"

const char *server_cert_buffer =
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIDjTCCAnUCFCx0tlN5M0jcBAGjCMKf2DAS0hZKMA0GCSqGSIb3DQEBCwUAMIGB\r\n"
    "MQswCQYDVQQGEwJDTjENMAsGA1UECAwEV3VYaTENMAsGA1UEBwwEV3VYaTEOMAwG\r\n"
    "A1UECgwFV0tKYXkxDjAMBgNVBAsMBVdLSmF5MRQwEgYDVQQDDAsqLndramF5LmNv\r\n"
    "bTEeMBwGCSqGSIb3DQEJARYPdW5yZWFsQHRlc3QuY29tMCAXDTIyMDcxMTA3NTcz\r\n"
    "MloYDzIxMjIwNjE3MDc1NzMyWjCBgTELMAkGA1UEBhMCQ04xDTALBgNVBAgMBFd1\r\n"
    "WGkxDTALBgNVBAcMBFd1WGkxDjAMBgNVBAoMBVdLSmF5MQ4wDAYDVQQLDAVXS0ph\r\n"
    "eTEUMBIGA1UEAwwLKi53a2pheS5jb20xHjAcBgkqhkiG9w0BCQEWD3VucmVhbEB0\r\n"
    "ZXN0LmNvbTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAL4KttV1Pz1T\r\n"
    "RTADsypnT9Gx/thb4C28900F9cFSmitHRLeWgZ/lEshZl08rXouHFYB8Pq6NBHeP\r\n"
    "akuv6M3kp227QcBHEIvwVbIBSfSJEMu1MgPd2AbwlQ2ZaUqIgtNQ/BzwqOYgaOaQ\r\n"
    "LvmjcIiRdLpaXWlrfZN/YMcLkovAAvglck+KppElbbtz78T/e1HJlYQNTM7AzpaT\r\n"
    "641lAZpoxR7GcyRnW4Te3nAsRELYcKVMoLC256OviuZCCPFd+ec1awa6Cqh8f9ww\r\n"
    "zmSj5y5inF8uUPIitGODLDOwn4gfLDIXEH5gf67u8tAtnllzhclJ/OeYdZoeVDzM\r\n"
    "lEWwTMSQRmcCAwEAATANBgkqhkiG9w0BAQsFAAOCAQEAFp8yP5XbR32BdQVraLsb\r\n"
    "gidEzuu3B8BMwlrWb64HiNbiozKFHC6uPsb/E0l8BedMaqQC3FseGAmIlTWX1KWy\r\n"
    "s6lVFtZa663xOG0EW8opWzLKSGgakgf6NtSNQBZPwXrc7nLxuVH06raYaM9OAcXV\r\n"
    "u4Nk0sGAQGc63khRHfuV+zfXqNHwxczM6sW/DvAj01UW5Ea6K7gmTW0h/fvbXoC0\r\n"
    "WsidBAEPdxExMPuUojQcHAtw96ojMnDliwDX5hKd+OJK3cLXKQa+8UUfiTPEexVl\r\n"
    "L4dteJUKD1u6ReftQvDW+2YUynVzkj8hmyv1JpTXK+9Q0xfmxVl22WgE+qkZHL+7\r\n"
    "TQ==\r\n"
    "-----END CERTIFICATE-----\r\n";

const char *server_key_buffer =
    "-----BEGIN RSA PRIVATE KEY-----\r\n"
    "MIIEogIBAAKCAQEAvgq21XU/PVNFMAOzKmdP0bH+2FvgLbz3TQX1wVKaK0dEt5aB\r\n"
    "n+USyFmXTytei4cVgHw+ro0Ed49qS6/ozeSnbbtBwEcQi/BVsgFJ9IkQy7UyA93Y\r\n"
    "BvCVDZlpSoiC01D8HPCo5iBo5pAu+aNwiJF0ulpdaWt9k39gxwuSi8AC+CVyT4qm\r\n"
    "kSVtu3PvxP97UcmVhA1MzsDOlpPrjWUBmmjFHsZzJGdbhN7ecCxEQthwpUygsLbn\r\n"
    "o6+K5kII8V355zVrBroKqHx/3DDOZKPnLmKcXy5Q8iK0Y4MsM7CfiB8sMhcQfmB/\r\n"
    "ru7y0C2eWXOFyUn855h1mh5UPMyURbBMxJBGZwIDAQABAoIBAGc7SrYJSqD1as/6\r\n"
    "MokGNcWi+txsjApMa8nbQvQQ+s4nmJxhlWhV9y39/MN0u5bvei6hTytiTtrjfMpA\r\n"
    "dCXj308sOTtJXyOlGefn61R6YDVH6DNRftfGODF69EcYgHhptYnC8PyQ/mrAR8Qz\r\n"
    "lB2bZd0U2Uk6qqxEtT1qe+COHQ7N16ChZ5YYwAebFnByPhPCq78QINHQoDQDknCT\r\n"
    "Xes2LfmevH0grirD2MfIbbOBLyznwsLlaQwYOrNrzYKLDcO5NgBYUHzVotqiMoA3\r\n"
    "vzU4YksGnjcV0euNGLgK5MSBxOpdNmcAJ5wBtMWGNElRjY1ogWPy92wk383vW9md\r\n"
    "kWvbMwECgYEA9TNW2HGnQ+m3u0rmuu+h6kOegUZzuR1YLO3giRVCXYlEwIZ1Ljxn\r\n"
    "NaW/7H2LoG9lqXVCoFdjJuqlFzEcPufEwGFgRRcLjusgxIe7B4xHbp/Ymsoucigi\r\n"
    "7S7LzylAV+iecdk1fghvh6tfKdA/W/q0z0SElRFsqSs3DCm9hKc21A8CgYEAxml0\r\n"
    "hdBU7d9Yz31e8kZOVNT//y290w2Dfkbh6D/4xrymPd1cwLPqWEhmqmkIc1+7l4s+\r\n"
    "k5HIfMBP9w40CVwgDq0SUN04GwzeszVQbaJlkOa8KP+ny/G/QHpXFnGxUHXeF//I\r\n"
    "/RNjkcArHrUc/NPU7ZODg84ZemE1gSQqKM8isCkCgYB3UTM6ghvF1W5dynX6k290\r\n"
    "AtGX0MOxWdE1k8/GhTzVLV3yXbuZ8zS6C10YZINUX8DVtETmp3+NSXNqlLBNABVj\r\n"
    "FD93f15Vfp9kYzQk2SNNdqU9tZLiZBuS1UnCFi3EWWL4vZzlJo+3MjJNs5ORW68u\r\n"
    "iQYHUAJTU78mwQ0DByeMCwKBgAFA3UmTHVY7WPZGlnj1VL1Ycx2Ljm1s4m3DyN2M\r\n"
    "ueeXfX1ajqFxAYP5QRzGeRUxf5/fc0+/VgLjvB2Va2K7wEAXe8wi+Z3CIQ4EwjNP\r\n"
    "GVEnA/1GUCsLpeekXjR4F2SoufRw2zYuDyz2h88z2bEHLYsqqWQFw0dwocPlFJcZ\r\n"
    "Z+CxAoGAOyCWhXLxcl3eOtOCsIyTLaKl3WgFaZ+DdF7e3zAqMB/W+ucCHeML0o3Q\r\n"
    "NocuRj/ivqEckG6Wd3pfUXPHEGUVlBKeEPQQK8akO4r0VOPH2IrKbxBpelq7vXKk\r\n"
    "ZqDVjvvNOutjNeky4HB6r0owCNcHPqcpIJfr9+FhChtVUX51WJA=\r\n"
    "-----END RSA PRIVATE KEY-----\r\n";

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
    opts.server_cert_buffer = server_cert_buffer;
    opts.server_key_buffer = server_key_buffer;

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
MSH_CMD_EXPORT(ssl_server_init, ssl server init);
