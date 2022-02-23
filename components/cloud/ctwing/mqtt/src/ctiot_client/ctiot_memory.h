#ifndef __CTIOT_MEMORY_
#define __CTIOT_MEMORY_

#include <stdio.h>
#include <string.h>
#include <stdlib.h> 

typedef   signed char  SDK_S8;
typedef unsigned char  SDK_U8;

typedef     short int  SDK_S16;
typedef unsigned short SDK_U16;

typedef   signed int   SDK_S32;
typedef unsigned int   SDK_U32;

typedef unsigned long long SDK_U64;

struct CTIOT_HEAP_NODE
{
    SDK_U8 used;
    void *memPtr;
    struct CTIOT_HEAP_NODE *next;
};

struct CTIOT_HEAP_HEAD
{
    SDK_U8 init;
    SDK_U32 maxSize;
    SDK_U32 blockSize;
    SDK_U32 blockNumber;
    void *startPtr;
    struct CTIOT_HEAP_NODE *list;
};

#define OS_MALLOC malloc
#define OS_FREE free

#if defined(USE_MUTEX)
typedef pthread_mutex_t T_MUTEX_ID;
#define OS_CREATE_MUTEX(MUTEX)  pthread_mutex_init(MUTEX,NULL)
#define OS_GET_MUTEX(MUTEX)              pthread_mutex_lock(MUTEX)
#define OS_PUT_MUTEX(MUTEX)              pthread_mutex_unlock(MUTEX)
#define OS_CLOSE_MUTEX(MUTEX)            pthread_mutex_destroy(MUTEX)
#endif

SDK_S16 ctiot_init_heap(SDK_U32 blockSize, SDK_U32 blockNumber);

void ctiot_free_all_heap(void);

void *ctiot_heap_malloc(SDK_U32 size);

void ctiot_heap_free(void* ptr);

#endif
