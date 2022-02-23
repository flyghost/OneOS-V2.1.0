/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * @file        mqtt_os.h
 * 
 * @brief       Header file for mqtt os interface.
 * 
 * @details     
 * 
 * @revision
 * Date         Author          Notes
 * 2020-12-10   OneOs Team      First Version
 ***********************************************************************************************************************
 */

#if !defined(__MQTT_LINUX_)
#define __MQTT_LINUX_

#if defined(WIN32_DLL) || defined(WIN64_DLL)
  #define DLLImport __declspec(dllimport)
  #define DLLExport __declspec(dllexport)
#elif defined(LINUX_SO)
  #define DLLImport extern
  #define DLLExport  __attribute__ ((visibility ("default")))
#else
  #define DLLImport
  #define DLLExport
#endif



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/errno.h>
#include <os_clock.h>
#include <os_task.h>
#include <os_timer.h>
#include <os_mutex.h>
#include <os_assert.h>

/* mqtt_os return value definitions */
#define FALSE  0
#define TRUE   1

typedef os_task_t *Thread;
typedef os_mutex_t Mutex;

typedef struct Timer
{
    os_tick_t xTicksToWait; /* tick wait setting for this timer */
    os_tick_t xTicksRecord; /* record the tick value */
    os_timer_t *xTimeOut;   /* ct_timer ref */
} Timer;

void TimerInit(Timer *);
void TimerRelease(Timer *);
char TimerIsExpired(Timer *);
void TimerCountdownMS(Timer *, unsigned int);
void TimerCountdown(Timer *, unsigned int);
int TimerLeftMS(Timer *);

typedef struct Network Network;

struct Network
{
    const char *pHostAddress;
    uint16_t port;
    uint16_t ca_crt_len;
    /* NULL: TCP connection, not NULL: SSL connection */
    const char *ca_crt;
    /* connection handle: 0 or (uintptr_t)(-1) not connection */
    uintptr_t handle;
    /* function pointer of recv mqtt data */
    int (*mqttread)(Network *, unsigned char *, int, int);
    /* function pointer of send mqtt data */
    int (*mqttwrite)(Network *, unsigned char *, int, int);
    /* function pointer of disconnect mqtt network */
    int (*disconnect)(Network *);
    /* function pointer of establish mqtt network */
    int (*connect)(Network *);
};

DLLExport int NetworkInit(Network *pNetwork, const char *host, uint16_t port, const char *ca_crt);
DLLExport int NetworkConnect(Network* n);
DLLExport void NetworkDisconnect(Network* n);

#endif
