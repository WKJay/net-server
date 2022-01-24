# TCP/TLS 服务器

## 简介
如果你厌倦了繁琐的网络服务器基础框架的搭建，不想每次实现一个网络服务器应用时还要花费大量的时间在连接的接入与断开、各种状态的处理，只想专注于处理应用数据，那就试用一下这个软件包吧！

该软件包接管了复杂的网络连接流程，将应用数据通过回调式的接口暴露给用户，使用户可以专注于应用层的逻辑处理。并且支持SSL/TLS服务器的搭建（目前对WolfSSL做了接口的移植）,使得用户仅需多配置几个参数，就能够和搭建一个TCP服务器一样完成SSL/TLS服务器的搭建。

### 特性
- TCP/TLS 服务器
- 多客户端，可自行设置最大能接受的客户端数量
- 回调式的数据接收，数据到来时自动调用，不影响其他逻辑的执行
- 简易的接口以及配置方式
- 全自动的连接管理机制，用户无须关注每个连接的状态处理
- 连接心跳机制，一定时间内未收到数据自动断开，防止伪连接占用资源
- 可创建多个服务器，各个服务器单独运行，互不干扰。

## 获取方式
使用 **net_server** 软件包需要在 RT-Thread 包管理软件中选中它，具体路径如下：

```
RT-Thread online packages --->
    IoT - internet of things --->
        [*] net_server:TCP/TLS server,support wolfssl
```

当激活 **net_server** 软件包后，可以激活 *Use TCL Server example* 选项将 TCP Server 的示例代码加入到工程。如果要支持 TLS 服务器功能，则激活 *Enable TLS support* 选项。

在 *Enable TLS support* 子选项中可以选择 *tls port file* 来使用对应的 TLS 包的支持（目前仅支持WolfSSL）,同样激活 *Use tls server example* 后可将TLS示例代码加入工程中。

## 使用说明
由于 **net_server** 为用户接管了服务器的运行逻辑，因此无论是TCP还是TLS服务器，用户都仅需以下流程即可建立对应的服务器应用：

1. 配置服务器参数
2. 创建服务器实例
3. 启动服务器

### 配置服务器参数
#### 参数概览
**net_server** 的服务器参数配置使用 `netserver_opt_t` 结构体来管理，该结构成员如下所示：

```C
typedef struct _netserver_opt {
    uint16_t listen_port;         // server listen port
    uint32_t data_pkg_max_size;   // max size of data package
    uint32_t max_conns;           // max connections
    uint32_t session_timeout;     // session timeout
    netserver_cb_t callback;      // callback functions
    thread_attrs_t thread_attrs;  // server thread attrs
#if NS_ENABLE_SSL
    const char *server_key;
    const char *server_cert;
    const char *ca_cert;
#endif
} netserver_opt_t;
```

成员详解：

|成员名|描述|
|-|-|
|listen_port|服务器端口号|
|data_pkg_max_size|一包数据的最大大小|
|max_conns|服务器能够接受的最大客户端数|
|session_timeout|一条连接的超时时间|
|callback|回调配置|
|thread_attrs|线程配置|
|server_key|**[SSL/TLS]** 服务器私钥|
|server_cert|**[SSL/TLS]** 服务器证书|
|ca_cert|**[SSL/TLS]** CA证书|

- `session_timeout` 指的是一条连接在该时间内未收到数据则会自动断开连接，避免了伪连接的存在。

- `callback`为`netserver_cb_t`类型的回调配置组，其中包括以下回调函数成员：

    ```C
    typedef struct _netserver_cb {
        void (*session_create_cb)(ns_session_t *session);
        void (*session_close_cb)(ns_session_t *session);
        int (*session_accept_cb)(ns_session_t *session);
        int (*data_readable_cb)(ns_session_t *session, void *data, int sz);
    #if NS_ENABLE_SSL
        int (*ssl_handshake_cb)(ns_session_t *session, void *cert_data,
                                int cert_size);
    #endif
    } netserver_cb_t;
    ```

    |成员名|描述|
    |-|-|
    |netserver_reset_cb|当 netserver 即将重启时被调用|
    |session_create_cb|当一条客户端连接被创建后被调用|
    |session_close_cb|当一条连接被关闭后被调用|
    |session_accept_cb|当一条连接被接入后被调用|
    |session_poll_cb|当触发轮询事件后被调用（数据可读/连接异常/select 超时）|
    |data_readable_cb|当一条连接上有数据到达后被调用|
    |ssl_handshake_cb|**[SSL/TLS]** 当一条连接进行SSL握手后被调用|

- `thread_attrs` 包含了 **net_server** 主线程的线程参数配置：
    |成员名|描述|
    |-|-|
    |stack_size|线程栈大小|
    |priority|线程优先级|
    |tick|时间片大小|


#### CallBack API 详解
**net_server** 的核心即基于回调的用户层处理方式，因此要用好该软件包必须了解其回调接口。

在了解其回调接口之前，首先要认识一个重要的结构：`ns_session_t`。该结构是 *net_server* 中一条连接（会话）的处理句柄，在回调接口被调用时会由 **net_server** 核心层传入该类型的一个参数 `session`，其包含有连接的一些属性，大部分时候用户无须关注其内部结构，只需在调用发送接口发送数据时传入该参数即可，但对于一些高级用户，可以使用其中的一个参数 `user_data` ,该参数是 `void *` 类型，用于可以给每条连接定义一些私有的数据，并且挂在该参数上，使用户可以对每一条连接进行一些独立的数据处理。

##### 1. 会话创建回调
`void (*session_create_cb)(ns_session_t *session);`

该接口会在一个客户端请求连接并且内核为其分配了一个会话结构后被调用，用于提示应用层当前有客户端请求接入，内核在调用该接口时会传入为当前连接创建的会话句柄。用户可以在此时为 `session->user_data` 创建一些应用层数据。

##### 2. 会话关闭回调
`void (*session_close_cb)(ns_session_t *session);`

该接口会在一个会话关闭后（此时还未释放资源）被内核调用，并且传入当前关闭的会话句柄，若用户在创建会话时在 `session->user_data` 上创建过应用层数据，则可在此时释放中的应用层数据。
##### 3. 会话接入回调
`int (*session_accept_cb)(ns_session_t *session);`

该接口会在一个客户端连接成功接入后被内核调用，并传入当前的会话句柄，用户可在此时获取会话句柄中的客户端信息(`session->cliaddr`)进行一些认证。

该接口有一个返回值，该返回值由用户控制，返回 **0** 表示正常接受该会话连接，若返回 **-1** 内核将立即断开该连接。结合上述的信息认证用户可以实现例如IP白名单功能。
##### 4. 数据接收回调
`int (*data_readable_cb)(ns_session_t *session, void *data, int sz);`

该接口会在内核接收到数据后调用，内核一次接收的数据最大长度由配置参数中的 `data_pkg_max_size` 设置。参数详解如下表：

|参数|描述|
|-|-|
|session|会话句柄|
|data|数据缓冲区首地址|
|sz|本次接收到数据的长度|
|返回|返回值暂时未起作用|

##### 5. [SSL/TLS] 会话进行SSL握手回调
`int (*ssl_handshake_cb)(ns_session_t *session, void *cert_data,int cert_size);`

该接口会在 SSL 握手成功后被调用，并且传入客户端证书及其长度，用户可以在此进行证书的进一步认证，或者获取证书中的信息。

|参数|描述|
|-|-|
|session|会话句柄|
|cert_data|客户端证书数据|
|cert_size|客户端证书长度|
|返回|返回值暂时未起作用|

### 创建服务器实例

用户配置好服务器参数后，即可调用下列接口创建一个服务器实例：

`netserver_mgr_t *netserver_create(netserver_opt_t *opts, uint32_t flag)`

|参数|描述|
|-|-|
|opts|配置参数|
|flag|服务器运行标志|
|返回|/|
|成功|返回服务器句柄|
|失败|NULL|

用户使用的服务器标志如下：
|标志|描述|
|-|-|
|NS_USE_SSL|标志该服务器是SSL/TLS服务器|
|NS_SSL_VERIFY_PEER|SSL/TLS模式下需要认证客户端证书|
|NS_SSL_FORCE_PEER_CERT|若没有客户端没有提供证书则强制断开连接|

### 启动服务器

用户创建好服务器后，即可调用下列接口启动服务器：

`int netserver_start(netserver_mgr_t *mgr)`

|参数|描述|
|-|-|
|mgr|服务器句柄|
|返回|/|
|成功|0|
|失败|-1|

一旦服务器成功启动，用户仅需关注自己注册的回调函数被调用以及在对应的时间发送想要发送的数据即可。

### 发送数据

用户可以在一个会话（连接）有效期间的任何时间调用如下接口向该会话（连接）发送数据：

`int netserver_write(ns_session_t *ns, void *data, int sz)`

|参数|描述|
|-|-|
|ns|会话句柄|
|data|发送的数据缓冲区首地址|
|sz|发送的数据长度|
|返回|/|
|成功|返回发送的数据长度|
|失败|0|
### 重启服务器

用户可能希望在某种情况下重启服务器（如更改端口号，或更改IP等），此时仅需调用如下接口：

`void netserver_restart(netserver_mgr_t *mgr);`

传入的参数为想要重启的服务器句柄。用户可以在调用该函数之前修改配置参数，从而实现服务器重启后更改运行参数如端口号等。

**重启服务器前核心层会自动断开所有连接**



## 代码示例
### TCP 服务器
```C
#include <rtthread.h>
#include <stdio.h>
#include "netserver.h"

static int netserver_readable_cb(ns_session_t *ns, void *data, int sz) {
    int ret = 0;
    ret = netserver_write(ns, data, sz);
    return ret;
}

int tcp_server_init(void) {
    netserver_opt_t opts;
    netserver_mgr_t *mgr = NULL;
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
        printf("create simple tcp server manager failed.\r\n");
        return -1;
    }

    /* start netserver */
    if (netserver_start(mgr) == 0) {
        printf("start simple tcp server on port %d.\r\n", opts.listen_port);
        return 0;
    } else {
        printf("start simple tcp server error.\r\n");
        return -1;
    }
}
MSH_CMD_EXPORT(tcp_server_init,tcp server init);

```
### TLS 服务器
```C
#include <rtthread.h>
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
MSH_CMD_EXPORT(ssl_server_init,ssl server init);
```

## 注意事项
WolfSSL 暂时不存在于 RT-Thread 软件包生态中，但是本软件包作者已将RTT适配入WolfSSL并存在于 WolfSSL 官方仓库中，用户可以直接下载 WolfSSL 官方包并参考其官方文档即可使用。 
## 联系人
- 作者：  WKJay
- 主页：  https://github.com/WKJay/net-server
- eMail: 1931048074@qq.com
