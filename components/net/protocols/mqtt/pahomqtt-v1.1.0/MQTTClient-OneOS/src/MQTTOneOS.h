/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the \"License\ you may not use this file except in compliance with 
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on 
 * an \"AS IS\" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the 
 * specific language governing permissions and limitations under the License.
 *
 * \@file        MQTTOneOS.h
 *
 * \@brief       socket port file for mqtt
 *
 * \@details     
 *
 * \@revision
 * Date         Author          Notes
 * 2020-06-08   OneOS Team      first version
 * 2020-12-29   OneOS Team      modify tls netowrk interface and add new mqtt net interface
 ***********************************************************************************************************************
 */

#if !defined(MQTTONEOS_H)
#define MQTTONEOS_H

#include <stdint.h>
#include <os_task.h>
#include <os_mutex.h>
#include <os_timer.h>

/* if recv pkg using recv task, need define MQTT_TASK */
//#define MQTT_TASK

/* MQTTOneOS return value definitions */
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

typedef struct Network Network;

struct Network
{
    /* if only use tcp */
	/* int my_socket;
	int (*mqttread) (Network*, unsigned char*, int, int);
	int (*mqttwrite) (Network*, unsigned char*, int, int);
	void (*disconnect) (Network*);
    int (*connect) (Network*); */

    const char *pHostAddress;
    uint16_t port;
    uint16_t ca_crt_len;
    /* NULL: TCP connection, not NULL: SSL connection */
    const char *ca_crt;
    /* Using PSK */
    const uint8_t *psk_identity;
    uint16_t psk_identity_len;    
    uint16_t psk_key_len;
    const uint8_t *psk_key;
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

void TimerInit(Timer *);
void TimerRelease(Timer *);
char TimerIsExpired(Timer *);
void TimerCountdownMS(Timer *, unsigned int);
void TimerCountdown(Timer *, unsigned int);
int TimerLeftMS(Timer *);

void MutexInit(Mutex *);
void MutexDeInit(Mutex *mutex);
int MutexLock(Mutex *);
int MutexUnlock(Mutex *);

#if defined(MQTT_TASK)
int ThreadStart(Thread *, void (*fn)(void *), void *arg);
#endif
#ifdef MQTT_USING_TLS_ONETLS
int MQTTNetworkInit(Network *pNetwork, const char *host, uint16_t port, const uint8_t *psk_identity, 
                    uint16_t psk_identity_len, const uint8_t *psk_key, uint16_t psk_key_len);
#else
int MQTTNetworkInit(Network *, const char *, uint16_t, const char *);
#endif
int MQTTNetworkConnect(Network *pNetwork);
void MQTTNetworkDisconnect(Network *pNetwork);
#endif
