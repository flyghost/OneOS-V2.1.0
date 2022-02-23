//// Copyright (c) Microsoft. All rights reserved.
//// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/xlogging.h"

//#include "os_kernel.h"
#include "os_task.h"
#include "os_timer.h"
#include "os_errno.h"
// DEFINE_ENUM_STRINGS(THREADAPI_RESULT, THREADAPI_RESULT_VALUES);

// typedef struct thread_arg_tag
//{
//    char *thread_name;
//    int stack_size;
//    int thread_priority;
//    int thread_time_tiumeslice;
//} thread_arg_tag_t;

// void os_task_exit(void);
// THREADAPI_RESULT ThreadAPI_Create(THREAD_HANDLE* threadHandle, THREAD_START_FUNC func, void* arg)
//{
//    THREADAPI_RESULT result;
//    if ((threadHandle == NULL) ||
//        (func == NULL))
//    {
//        result = THREADAPI_INVALID_ARG;
//        LogError("(result = %s)", ENUM_TO_STRING(THREADAPI_RESULT, result));
//    }
//    else
//    {
//        thread_arg_tag_t *thread_arg = arg;
//        *threadHandle = os_task_create(thread_arg->thread_name ,
//                                         (void (*)(void *))func,
//                                         NULL,
//                                         thread_arg->stack_size,
//                                         thread_arg->thread_priority);

//        if(*threadHandle == NULL)
//        {
//            result = THREADAPI_ERROR;
//            LogError("(result = %s)", ENUM_TO_STRING(THREADAPI_RESULT, result));
//        }
//        else
//        {
//            result = THREADAPI_OK;
//        }
//    }

//    return result;
//}

// THREADAPI_RESULT ThreadAPI_Join(THREAD_HANDLE threadHandle, int *res)
//{
//    return OS_EOK;
//}

// void ThreadAPI_Exit(int res)
//{
//    os_task_exit();
//}

void ThreadAPI_Sleep(unsigned int milliseconds)
{
    os_task_msleep(milliseconds);
}
