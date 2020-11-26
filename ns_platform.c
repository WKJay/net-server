/*************************************************
 Copyright (c) 2020
 All rights reserved.
 File name:     ns_platform.c
 Description:   
 History:
 1. Version:    
    Date:       2020-11-26
    Author:     wangjunjie
    Modify:     
*************************************************/
#include "netserver.h"

#include <rtthread.h>
#define NS_THREAD_STACK_SIZE 8192
#define NS_THREAD_PRIORITY   10
#define NS_THREAD_TICK       5

int ns_thread_start(void *(*entry)(void *), void *param) {
    rt_thread_t tid = rt_thread_create("netserver", entry, param,
                                       NS_THREAD_STACK_SIZE,
                                       NS_THREAD_PRIORITY,
                                       NS_THREAD_TICK);
    if (tid) {
        if (rt_thread_startup(tid) == RT_EOK) {
            return 0;
        } else {
            return -1;
        }
        return 0;
    } else {
        return -1;
    }
}
