#ifndef __NS_TYPES_H
#define __NS_TYPES_H

#include <string.h>
#include <stdint.h>
#include "ns_cfg.h"

#define NS_MEMCPY(d, s, l)   memcpy((d), (s), (l))
#define NS_MEMSET(b, c, l)   memset((b), (c), (l))
#define NS_MEMCMP(s1, s2, n) memcmp((s1), (s2), (n))

#include <stdlib.h>
#define NS_MALLOC(s) malloc((size_t)(s))
#define NS_FREE(p)            \
    {                         \
        void* xp = (p);       \
        if ((xp)) free((xp)); \
    }
#define NS_REALLOC(p, n, h, t) realloc((p), (size_t)(n))
#define NS_CALLOC(c, s)        calloc(c, s)

#include <stdio.h>
#define NS_LOG(...)          \
    printf("[netserver]: "); \
    printf(__VA_ARGS__);     \
    printf("\r\n");
    

#endif /* __NS_TYPES_H */
