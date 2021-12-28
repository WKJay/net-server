#ifndef __NS_CFG_H
#define __NS_CFG_H

#if defined(NET_SERVER_USING_TLS)
    #define NS_ENABLE_SSL                1
#else
    #define NS_ENABLE_SSL                0
#endif

#define NS_DATA_PKG_MAX_SIZE_DEFAULT 2048

/**
 *  Net server default thread config
 * */
#define NS_THREAD_STACK_SIZE_DEFAULT 4096
#define NS_THREAD_PRIORITY_DEFAULT   10
#define NS_THREAD_TICK_DEFAULT       5

#endif /* __NS_CFG_H */
