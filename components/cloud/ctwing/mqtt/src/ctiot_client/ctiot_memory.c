#include "ctiot_memory.h"

struct CTIOT_HEAP_HEAD ctiotHeapHead = {0};
#if defined(USE_MUTEX)
T_MUTEX_ID mutex;
#endif

SDK_S16 ctiot_init_heap(SDK_U32 blockSize, SDK_U32 blockNumber)
{
    SDK_S16 result = 0;
    if(ctiotHeapHead.init == 1)
    {
        return -1;
    }
#if defined(USE_MUTEX)
    OS_CREATE_MUTEX (&mutex);
#endif
#if defined(USE_MUTEX)
    OS_GET_MUTEX(&mutex);
#endif   
    ctiotHeapHead.init = 1;
    ctiotHeapHead.blockSize = blockSize;
    ctiotHeapHead.blockNumber = blockNumber;
    ctiotHeapHead.maxSize = blockSize * blockNumber;
    ctiotHeapHead.startPtr = (void *)OS_MALLOC(ctiotHeapHead.maxSize);
    SDK_U32 i = 0;
    struct CTIOT_HEAP_NODE *list_tail = NULL;
    if(ctiotHeapHead.startPtr == NULL)
    {
        result = -1;
        goto exit;
    }
    memset(ctiotHeapHead.startPtr,0,ctiotHeapHead.maxSize);
    list_tail = ctiotHeapHead.list;
    for(i = 0 ; i < blockNumber ; i++)
    {
        struct CTIOT_HEAP_NODE * ptr = (struct CTIOT_HEAP_NODE *)OS_MALLOC(sizeof(struct CTIOT_HEAP_NODE));
        if(ptr == NULL)
        {
            result = -1;
            goto exit;
        }
        ptr->used = 0;
        //ptr->memPtr = ctiotHeapHead.startPtr + (i * ctiotHeapHead.blockSize);/* modify by OneOS Team, solve compile error */
        ptr->memPtr = (unsigned char *)ctiotHeapHead.startPtr + (i * ctiotHeapHead.blockSize);
        ptr->next = NULL;
        if (list_tail == NULL)
        {
            ctiotHeapHead.list = ptr;
            list_tail = ptr;
        }
        else
        {
            list_tail->next = ptr;
            list_tail = list_tail->next;
        }
    }
exit:
#if defined(USE_MUTEX)
    OS_PUT_MUTEX(&mutex);
#endif
    //if(result == 1)/* modify by OneOS Team, solve bug */
    if(result == -1)
    {
        ctiot_free_all_heap();
    }
    return result;
}

static void ctiot_free_heap_list()
{
    struct CTIOT_HEAP_NODE *listHead = ctiotHeapHead.list;
    struct CTIOT_HEAP_NODE *node = NULL;
    while (listHead != NULL)
    {
        node = listHead;
        listHead = listHead->next;
        OS_FREE(node);
        node = NULL;
    }
    ctiotHeapHead.list = NULL;
}

void ctiot_free_all_heap(void)
{
    if(ctiotHeapHead.init != 1)
    {
        return;
    }
#if defined(USE_MUTEX)
    OS_GET_MUTEX(&mutex);
#endif
    ctiot_free_heap_list();
    ctiotHeapHead.blockSize = 0;
    ctiotHeapHead.blockNumber = 0;
    ctiotHeapHead.maxSize = 0;
    //OS_FREE(ctiotHeapHead.startPtr);/* modify by OneOS Team, solve bug */
    if(ctiotHeapHead.startPtr != NULL)
    {
        OS_FREE(ctiotHeapHead.startPtr);
    }
    ctiotHeapHead.startPtr = NULL;
    ctiotHeapHead.init = 0;
#if defined(USE_MUTEX)
    OS_PUT_MUTEX(&mutex);
    OS_CLOSE_MUTEX(&mutex);
#endif
}

void *ctiot_heap_malloc(SDK_U32 size)
{
    if(ctiotHeapHead.init != 1)
    {
        return NULL;
    }
    struct CTIOT_HEAP_NODE *node = ctiotHeapHead.list;
    SDK_U32 i = 0;
#if defined(USE_MUTEX)
    OS_GET_MUTEX(&mutex);
#endif
    while(node != NULL && i < ctiotHeapHead.blockNumber)
    {
        if(node->used == 0)
        {
            node->used = 1;
        #if defined(USE_MUTEX)
            OS_PUT_MUTEX(&mutex);
        #endif
            return node->memPtr;
        }
        node = node->next;
        i++;
    }
#if defined(USE_MUTEX)
    OS_PUT_MUTEX(&mutex);
#endif
    return NULL;
}

void ctiot_heap_free(void* ptr)
{
    if(ctiotHeapHead.init != 1)
    {
        return;
    }
    struct CTIOT_HEAP_NODE *node = ctiotHeapHead.list;
    SDK_U32 i = 0;
#if defined(USE_MUTEX)
    OS_GET_MUTEX(&mutex);
#endif
    while(node != NULL && i < ctiotHeapHead.blockNumber)
    {
        if(node->memPtr == ptr)
        {
            node->used = 0;
            memset(node->memPtr,0,ctiotHeapHead.blockSize);
        #if defined(USE_MUTEX)
            OS_PUT_MUTEX(&mutex);
        #endif
            return;
        }
        node = node->next;
        i++;
    }
#if defined(USE_MUTEX)
    OS_PUT_MUTEX(&mutex);
#endif
}
