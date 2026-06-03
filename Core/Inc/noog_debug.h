#ifndef __NOOG_DEBUG_H
#define __NOOG_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#ifndef NOOG_DEBUG_LOG_ENABLED
#define NOOG_DEBUG_LOG_ENABLED 1
#endif

#if NOOG_DEBUG_LOG_ENABLED
#define NOOG_LOG(tag, fmt, ...) printf("[" tag "] " fmt "\r\n", ##__VA_ARGS__)
#else
#define NOOG_LOG(tag, fmt, ...) do { } while (0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __NOOG_DEBUG_H */
