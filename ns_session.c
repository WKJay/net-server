/*************************************************
 Copyright (c) 2020
 All rights reserved.
 File name:     ns_session.c
 Description:   
 History:
 1. Version:    
    Date:       2020-11-26
    Author:     wangjunjie
    Modify:     
*************************************************/
#include "ns_session.h"

/**
 * Name:    ns_session_create
 * Brief:   create netserber session
 * Input:   
 *  @flag: session flag 
 *         NS_USE_TLS : use tls connection
 * Output:  netserver manager handler , NULL if create failed
 */
ns_session_t *ns_session_create(uint32_t flag) {
    ns_session_t *session = NS_CALLOC(1, sizeof(ns_session_t));
    if (session == NULL) {
        NS_LOG("no memory for ns session");
        return NULL;
    }
    session->flag = flag;
    return session;
}


