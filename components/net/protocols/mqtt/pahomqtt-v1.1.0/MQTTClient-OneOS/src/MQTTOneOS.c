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
 * @file        MQTTOneOS.c
 *
 * @brief       socket port file for mqtt
 *
 * @details     
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-08   OneOS Team      first version
 * 2020-12-29   OneOS Team      modify tls netowrk interface and add new mqtt net interface
 ***********************************************************************************************************************
 */

#include <oneos_config.h>
#include <stdio.h>
#include <string.h>
#include <os_task.h>
#include <os_clock.h>
#include <os_errno.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/errno.h>
#include <os_assert.h>
#include <dlog.h>
#include "MQTTOneOS.h"
#ifdef MQTT_USING_TLS
#if defined(MQTT_USING_TLS_MBEDTLS)
#include "mbedtls/error.h"
#include "mq_tls_mbedtls.h"
#elif defined MQTT_USING_TLS_ONETLS
#include "mq_tls_onetls.h"
#endif
#endif


#define MQ_NET_TAG   "MQ_NET"

#define U32_DIFF(a, b) (((a) >= (b)) ? ((a) - (b)) : (((a) + ((b) ^ 0xFFFFFFFF) + 1)))

/**
 ***********************************************************************************************************************
 * @brief           This function start a thread.
 *
 * @param[in]       thread        thread to create.
 * @param[in]       fn            function to run.
 * @param[in]       arg           arguments.
 *
 * @return          Return status.
 * @retval          TRUE          create success.
 * @retval          FALSE         create failed.
 ***********************************************************************************************************************
 */
#if defined(MQTT_TASK)
int ThreadStart(Thread *thread, void (*fn)(void *), void *arg)
{
    os_task_t  *tid = NULL;
    os_uint32_t stack_size = 1280;
    char        task_name[] = "MQTTTask";

    tid = os_task_create(task_name,
                         fn, (void *)arg,
                         stack_size,
                         OS_TASK_PRIORITY_MAX / 3);
    if (tid)
    {
        *thread = tid;
        os_task_startup(tid);
        return TRUE;
    }

    return FALSE;
}
#endif

/**
 ***********************************************************************************************************************
 * @brief           This function initialize mutex.
 *
 * @param[in]       mutex        mutex to initialize.
 *
 ***********************************************************************************************************************
 */
void MutexInit(Mutex *mutex)
{
    os_mutex_init(mutex, "mqtt_lock", OS_FALSE);
}

/**
 ***********************************************************************************************************************
 * @brief           This function deinitialize mutex.
 *
 * @param[in]       mutex        mutex to deinitialize.
 *
 ***********************************************************************************************************************
 */
void MutexDeInit(Mutex *mutex)
{
    os_mutex_deinit(mutex);
}

/**
 ***********************************************************************************************************************
 * @brief           This function take mutex.
 *
 * @param[in]       mutex        mutex to lock.
 *
 * @return          Return status.
 * @retval          TRUE         lock success.
 * @retval          FALSE        lock failed.
 ***********************************************************************************************************************
 */
int MutexLock(Mutex *mutex)
{
    os_err_t result;

    result = os_mutex_lock(mutex, OS_WAIT_FOREVER);
    if (result != OS_EOK)
    {
        return FALSE;
    }

    return TRUE;
}

/**
 ***********************************************************************************************************************
 * @brief           This function release mutex.
 *
 * @param[in]       mutex        mutex to unlock.
 *
 * @return          Return status.
 * @retval          TRUE         unlock success.
 * @retval          FALSE        unlock failed.
 ***********************************************************************************************************************
 */
int MutexUnlock(Mutex *mutex)
{
    os_err_t result;

    result = os_mutex_unlock(mutex);
    if (result != OS_EOK)
    {
        return FALSE;
    }

    return TRUE;
}

/**
 ***********************************************************************************************************************
 * @brief           This function set timer callback.
 ***********************************************************************************************************************
 */
void TimerCallback(void *parameter)
{
    /*Becareful: do not include any debug printf code.*/
}

/**
 ***********************************************************************************************************************
 * @brief           This function set timer parameter.
 *
 * @param[in]       timer        timer to set.
 * @param[in]       xTicksToWait ticks to wait.
 *
 ***********************************************************************************************************************
 */
void TimerSetTimeOutState(Timer *timer, os_tick_t xTicksToWait)
{
    os_err_t    status;
    os_tick_t   set_ticktowait = xTicksToWait;

    OS_ASSERT(NULL != timer->xTimeOut);

    os_timer_stop(timer->xTimeOut);
    os_timer_set_timeout_ticks(timer->xTimeOut, set_ticktowait);
    status = os_timer_start(timer->xTimeOut);
    OS_ASSERT(OS_EOK == status);

    timer->xTicksToWait = xTicksToWait;
    timer->xTicksRecord = os_tick_get();
}

/**
 ***********************************************************************************************************************
 * @brief           This function check whether timeout happened.
 *
 * @param[in]       timer        timer to check.
 *
 * @return          Return status.
 * @retval          TRUE         timeout.
 * @retval          FALSE        not timeout or error happened.
 ***********************************************************************************************************************
 */
int TimerCheckForTimeOut(Timer *timer)
{
    os_tick_t tick_now;
    os_tick_t tick_pre;
    os_tick_t tick_diff;

    if (NULL == timer)
        return FALSE;

    if (os_timer_is_active(timer->xTimeOut)) /*timer is not out*/
    {
        tick_now = os_tick_get();
        tick_pre = timer->xTicksRecord;
        tick_diff = U32_DIFF(tick_now, tick_pre);
        timer->xTicksRecord = tick_now;

        if (tick_diff < timer->xTicksToWait)
        {
            timer->xTicksToWait -= tick_diff;
            return FALSE;
        }
        else
        {
            timer->xTicksToWait = 0;
            os_timer_stop(timer->xTimeOut);
            return TRUE;
        }
    }
    else /*timer out happen*/
    {
        os_timer_stop(timer->xTimeOut);
        timer->xTicksToWait = 0;
        return TRUE;
    }
}

/**
 ***********************************************************************************************************************
 * @brief           This function set timer in milliseconds.
 *
 * @param[in]       timer        timer to check.
 * @param[in]       timeout_ms   time in milliseconds to set.
 ***********************************************************************************************************************
 */
void TimerCountdownMS(Timer *timer, unsigned int timeout_ms)
{
    os_tick_t xTicksToWait;

    xTicksToWait = os_tick_from_ms(timeout_ms); /* convert milliseconds to ticks */
    TimerSetTimeOutState(timer, xTicksToWait);  /* Record the time at which this function was entered. */
}

/**
 ***********************************************************************************************************************
 * @brief           This function set timer in seconds.
 *
 * @param[in]       timer        timer to check.
 * @param[in]       timeout      time in seconds to set.
 ***********************************************************************************************************************
 */
void TimerCountdown(Timer *timer, unsigned int timeout)
{
    TimerCountdownMS(timer, timeout * 1000);
}

/**
 ***********************************************************************************************************************
 * @brief           This function returns time left in milliseconds.
 *
 * @param[in]       timer        timer to check.
 *
 * @return          time left.
 * @retval          int          milliseconds.
 ***********************************************************************************************************************
 */
int TimerLeftMS(Timer *timer)
{
    return (timer->xTicksToWait * (1000 / OS_TICK_PER_SECOND));
}

/**
 ***********************************************************************************************************************
 * @brief           This function check whether timeout happened.
 *
 * @param[in]       timer        timer to check.
 *
 * @return          Return status.
 * @retval          TRUE         timeout.
 * @retval          others       not timeout or error happened.
 ***********************************************************************************************************************
 */
char TimerIsExpired(Timer *timer)
{
    return TimerCheckForTimeOut(timer);
}

/**
 ***********************************************************************************************************************
 * @brief           This function initialize timer.
 *
 * @param[in]       timer        timer to check.
 ***********************************************************************************************************************
 */
void TimerInit(Timer *timer)
{
    OS_ASSERT(NULL != timer);
    /*creat timer*/
    memset(&timer->xTimeOut, '\0', sizeof(timer->xTimeOut));
    timer->xTimeOut = os_timer_create("timer",
                                      TimerCallback,
                                      NULL,
                                      0,
                                      OS_TIMER_FLAG_ONE_SHOT);
    OS_ASSERT(NULL != timer->xTimeOut);

    timer->xTicksToWait = 0;
    timer->xTicksRecord = os_tick_get();
}

/**
 ***********************************************************************************************************************
 * @brief           This function release timer.
 *
 * @param[in]       timer        timer to check.
 ***********************************************************************************************************************
 */
void TimerRelease(Timer *timer)
{
    /*release timer*/
    if (NULL != timer->xTimeOut)
    {
        os_timer_stop(timer->xTimeOut);
        os_timer_destroy(timer->xTimeOut);
    }

    timer->xTicksToWait = 0;
    timer->xTicksRecord = os_tick_get();
    timer->xTimeOut = NULL;
}

#ifdef MQTT_USING_TLS
#if defined(MQTT_USING_TLS_MBEDTLS)
static uintptr_t ssl_establish(const char *host, uint16_t port, const char *ca_crt, uint32_t ca_crt_len)
{
    char        port_str[6] = {0};
    uintptr_t   tls_session_ptr = (uintptr_t)NULL;

    if (host == NULL || ca_crt == NULL)
    {
        LOG_E(MQ_NET_TAG, "input params are NULL, abort");
        return (uintptr_t)NULL;
    }

    if (!strlen(host) || (strlen(host) < 8))
    {
        LOG_E(MQ_NET_TAG, "invalid host: '%s'(len=%d), abort", host, (int)strlen(host));
        return (uintptr_t)NULL;
    }

    sprintf(port_str, "%u", port);
    if (0 != mq_tls_network_mbedtls_establish(&tls_session_ptr, host, port_str, ca_crt, NULL, NULL, NULL))
    {
        return (uintptr_t)NULL;
    }

    return (uintptr_t)(tls_session_ptr);
}

static int mqtt_connect_ssl(Network *pNetwork)
{
    if (NULL == pNetwork)
    {
        LOG_E(MQ_NET_TAG, "network is null");
        return -1;
    }

    pNetwork->handle = (intptr_t)ssl_establish(pNetwork->pHostAddress, 
                                                pNetwork->port, 
                                                pNetwork->ca_crt, 
                                                pNetwork->ca_crt_len + 1);
    if (0 != pNetwork->handle)
    {
        return 0;
    }
    else
    {
        /* The space has been freed */
        return -1;
    }
}

static int mqtt_disconnect_ssl(Network *pNetwork)
{
    if (NULL == pNetwork)
    {
        LOG_E(MQ_NET_TAG, "network is null");
        return -1;
    }

    mq_tls_network_mbedtls_close((mq_tls_session_t **)&(pNetwork->handle));
    pNetwork->handle = (uintptr_t)NULL; /* must have */
    LOG_I(MQ_NET_TAG, "MbedTLS connection close success.");

    return 0;
}

#elif defined(MQTT_USING_TLS_ONETLS)
static uint16_t tmp_psk_key_len;
const static uint8_t *tmp_psk_key;

static uint32_t onetls_get_psk(const uint8_t *hint, uint32_t hint_len, uint8_t *key, uint32_t max_key_len, uint32_t *key_len)
{
    if (max_key_len < tmp_psk_key_len) {
        return ONETLS_FAIL;
    }

    memcpy(key, tmp_psk_key, tmp_psk_key_len);
    *key_len = tmp_psk_key_len;
    return ONETLS_SUCCESS;
}

static int mqtt_connect_ssl(Network *pNetwork)
{
    if (NULL == pNetwork) {
        LOG_E(MQ_NET_TAG, "network is null");
        return -1;
    }

    if (pNetwork->psk_key == NULL || pNetwork->psk_identity == NULL) {
        LOG_E(MQ_NET_TAG, "networkd params are NULL, abort");
        return -1;
    }

    tmp_psk_key = pNetwork->psk_key;
    tmp_psk_key_len = pNetwork->psk_key_len;

    pNetwork->handle = (intptr_t)mq_tls_network_onetls_establish(pNetwork->pHostAddress, 
                                                                pNetwork->port, 
                                                                pNetwork->psk_identity, 
                                                                pNetwork->psk_identity_len,
                                                                onetls_get_psk);
    if (0 != pNetwork->handle) {
        return 0;
    } else {
        /* The space has been freed */
        return -1;
    }
}

static int mqtt_disconnect_ssl(Network *pNetwork)
{
    if (NULL == pNetwork)
    {
        LOG_E(MQ_NET_TAG, "network is null");
        return -1;
    }

    mq_tls_network_onetls_close((mq_tls_session_t *)(pNetwork->handle));
    pNetwork->handle = (uintptr_t)NULL; /* must have */
    LOG_I(MQ_NET_TAG, "OneTLS connection close success.");

    return 0;
}

#endif

#else
static int mqtt_connect_tcp(Network *pNetwork)
{
    int                 fd = 0;
    int                 retVal = -1;
    struct sockaddr_in  sAddr;
    struct hostent     *host_entry = NULL;
    long                socket_mode = 1; /* non-blocking */

    if (NULL == pNetwork)
    {
        LOG_E(MQ_NET_TAG, "MQTT pNetwork is null");
        return -1;
    }

    if ((host_entry = gethostbyname(pNetwork->pHostAddress)) == NULL)
    {
        LOG_E(MQ_NET_TAG, "dns parse error!");
        goto exit;
    }

    memset(&sAddr, 0, sizeof(struct sockaddr_in));
    sAddr.sin_family = AF_INET;
    sAddr.sin_port = htons(pNetwork->port);
    sAddr.sin_addr = *(struct in_addr *)host_entry->h_addr_list[0];

    if ((fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        goto exit;

    if ((retVal = connect(fd, (struct sockaddr *)&sAddr, sizeof(sAddr))) < 0)
    {
        closesocket(fd);
        goto exit;
    }

#if defined(BSD_USING_MOLINK)
    /*  Molink not support ioctlsocket, but molink default is non-blocking. */
    (void) socket_mode;
#elif defined(BSD_USING_LWIP)
    /* Set socket non-blocking mode. */
    if ((retVal = ioctlsocket(fd, FIONBIO, &socket_mode)) < 0 )
    {
        closesocket(fd);
        goto exit;
    }
#endif

    pNetwork->handle = (uintptr_t)fd;

exit:
    return retVal;
}

static int mqtt_disconnect_tcp(Network *pNetwork)
{
    if ((uintptr_t)(-1) == pNetwork->handle)
    {
        LOG_E(MQ_NET_TAG, "MQTT_Network->handle = -1");
        return -1;
    }
	closesocket(pNetwork->handle);
    pNetwork->handle = (uintptr_t)(-1);
    LOG_I(MQ_NET_TAG, "TCP connection close success.");

    return 0;
}
#endif

static int MQTT_net_connect(Network *pNetwork)
{
    int ret = 0;
#ifdef MQTT_USING_TLS
    #if defined(MQTT_USING_TLS_MBEDTLS)
        if (NULL != pNetwork->ca_crt)
        {
            ret = mqtt_connect_ssl(pNetwork);
        }
    #elif defined(MQTT_USING_TLS_ONETLS)
        if (NULL != pNetwork->psk_identity && NULL != pNetwork->psk_key) 
        {
            ret = mqtt_connect_ssl(pNetwork);
        }
    #endif
#else
    if (NULL == pNetwork->ca_crt)
    {
        ret = mqtt_connect_tcp(pNetwork);
    }
#endif
    else
    {
        ret = -1;
        LOG_E(MQ_NET_TAG, "no method match!");
    }

    return ret;
}

static int MQTT_net_disconnect(Network *pNetwork)
{
    int ret = 0;
#ifdef MQTT_USING_TLS
    #if defined(MQTT_USING_TLS_MBEDTLS)
        if (NULL != pNetwork->ca_crt)
        {
            ret = mqtt_disconnect_ssl(pNetwork);
        }
    #elif defined(MQTT_USING_TLS_ONETLS)
        if (NULL != pNetwork->psk_identity && NULL != pNetwork->psk_key) 
        {
            ret = mqtt_disconnect_ssl(pNetwork);
        }
    #endif        
#else
    if (NULL == pNetwork->ca_crt)
    {
        ret = mqtt_disconnect_tcp(pNetwork);
    }
#endif
    else
    {
        ret = -1;
        LOG_E(MQ_NET_TAG, "no method match!");
    }

    return ret;
}

#ifdef MQTT_USING_TLS
#if defined(MQTT_USING_TLS_MBEDTLS)
static int mqtt_read_ssl(Network *pNetwork, unsigned char *buffer, int len, int timeout_ms)
{
    os_tick_t   xTicksToWait = os_tick_from_ms(timeout_ms); /* convert milliseconds to ticks */
    Timer       xTimeOut = {0, 0, NULL};
    int         recvLen = 0;
    int         rc = 0;
    int         xMsToWait = 0;

    xMsToWait = timeout_ms;

    TimerInit(&xTimeOut);
    TimerSetTimeOutState(&xTimeOut, xTicksToWait); /* Record the time at which this function was entered. */

    static int net_status = 0;
    char err_str[33];

    mq_tls_session_t *session_ptr = (mq_tls_session_t *)pNetwork->handle;

    do
    {
        xTicksToWait = xTimeOut.xTicksToWait;
        xMsToWait = xTicksToWait * (1000 / OS_TICK_PER_SECOND);

        mbedtls_ssl_conf_read_timeout(&(session_ptr->config), xMsToWait);
        rc = mq_tls_network_mbedtls_read((mq_tls_session_t *)pNetwork->handle, 
                                         (unsigned char *)(buffer + recvLen), 
                                         len - recvLen);
        if (rc > 0)
        {
            recvLen += rc;
            net_status = 0;
        }
        else if (rc == 0)
        {
            /* if ret is 0 and net_status is -2, indicate the connection is closed during last call */
            if (net_status == -2)
                recvLen = net_status;
            break;
        }
        else
        {
            if (MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY == rc)
            {
                mbedtls_strerror(rc, err_str, sizeof(err_str));
                LOG_E(MQ_NET_TAG, "ssl recv error: code = %d, err_str = '%s'", rc, err_str);
                net_status = -2; /* connection is closed */
                recvLen = net_status;
                break;
            }
            else if ((MBEDTLS_ERR_SSL_TIMEOUT == rc)
                     || (MBEDTLS_ERR_SSL_CONN_EOF == rc)
                     || (MBEDTLS_ERR_SSL_SESSION_TICKET_EXPIRED == rc)
                     || (MBEDTLS_ERR_SSL_NON_FATAL == rc)) 
           {
                /* read already complete */
                /* if call mbedtls_ssl_read again, it will return 0 (means EOF) */
            }
            else
            {
                mbedtls_strerror(rc, err_str, sizeof(err_str));
                LOG_E(MQ_NET_TAG, "ssl recv error: code = %d, err_str = '%s'", rc, err_str);
                net_status = -1;
                recvLen = net_status;
                break;
            }
        }

    } while ((recvLen < len) && (TimerCheckForTimeOut(&xTimeOut) == FALSE));

    TimerRelease(&xTimeOut);

    return recvLen;
}

static int mqtt_write_ssl(Network *pNetwork, unsigned char *buffer, uint32_t len, uint32_t timeout_ms)
{
    os_tick_t   xTicksToWait = os_tick_from_ms(timeout_ms); /* convert milliseconds to ticks */
    Timer       xTimeOut = {0, 0, NULL};
    int         sentLen = 0;
    int         rc = 0;

    TimerInit(&xTimeOut);
    TimerSetTimeOutState(&xTimeOut, xTicksToWait); /* Record the time at which this function was entered. */

    do
    {
        xTicksToWait = xTimeOut.xTicksToWait;
        rc = mq_tls_network_mbedtls_write((mq_tls_session_t *)pNetwork->handle, 
                                          (const unsigned char *)(buffer + sentLen), 
                                          len - sentLen);
        if (rc > 0)
        {
            sentLen += rc;
        }
        else if (rc < 0)
        {
            if (rc != MBEDTLS_ERR_SSL_WANT_READ && rc != MBEDTLS_ERR_SSL_WANT_WRITE)
            {
                LOG_E(MQ_NET_TAG, "mbedtls_ssl_write returned -0x%x", -rc);
                sentLen = rc;
                break;
            }
        }
    } while ((sentLen < len) && (TimerCheckForTimeOut(&xTimeOut) == FALSE));

    TimerRelease(&xTimeOut);

    return sentLen;
}

#elif defined(MQTT_USING_TLS_ONETLS)
static int mqtt_read_ssl(Network *pNetwork, unsigned char *buffer, int len, int timeout_ms)
{
    os_tick_t   xTicksToWait = os_tick_from_ms(timeout_ms); /* convert milliseconds to ticks */
    Timer       xTimeOut = {0, 0, NULL};
    int         recvLen = 0;
    int         rc = 0;
    int         xMsToWait = 0;

    xMsToWait = timeout_ms;

    TimerInit(&xTimeOut);
    TimerSetTimeOutState(&xTimeOut, xTicksToWait); /* Record the time at which this function was entered. */

//    mq_tls_session_t *session_ptr = (mq_tls_session_t *)pNetwork->handle;

    do
    {
        xTicksToWait = xTimeOut.xTicksToWait;
        xMsToWait = xTicksToWait * (1000 / OS_TICK_PER_SECOND);

        onetls_socket_timeout(((mq_tls_session_t *)pNetwork->handle)->ctx, xMsToWait);
        rc = mq_tls_network_onetls_read((mq_tls_session_t *)pNetwork->handle, 
                                         (unsigned char *)(buffer + recvLen), 
                                         len - recvLen);
        if (rc > 0){
            recvLen += rc;
        } else if (rc == -2) { /* again */
            continue;
        } else if (rc == -3) { /* timout */
            /* read already complete */
            break;           
        } else {
            recvLen = -1;
            break;
        }
    } while ((recvLen < len) && (TimerCheckForTimeOut(&xTimeOut) == FALSE));

    TimerRelease(&xTimeOut);

    return recvLen;
}

static int mqtt_write_ssl(Network *pNetwork, unsigned char *buffer, uint32_t len, uint32_t timeout_ms)
{
    os_tick_t   xTicksToWait = os_tick_from_ms(timeout_ms); /* convert milliseconds to ticks */
    Timer       xTimeOut = {0, 0, NULL};
    int         sentLen = 0;
    int         rc = 0;

    TimerInit(&xTimeOut);
    TimerSetTimeOutState(&xTimeOut, xTicksToWait); /* Record the time at which this function was entered. */

    do
    {
        xTicksToWait = xTimeOut.xTicksToWait;
        rc = mq_tls_network_onetls_write((mq_tls_session_t *)pNetwork->handle, 
                                          (const unsigned char *)(buffer + sentLen), 
                                          len - sentLen);
        if (rc > 0) {
            sentLen += rc;
        }
        else if (rc < 0) {
            sentLen = rc;
            break;
        }
    } while ((sentLen < len) && (TimerCheckForTimeOut(&xTimeOut) == FALSE));

    TimerRelease(&xTimeOut);

    return sentLen;
}

#endif

#else

static int mqtt_read_tcp(Network *pNetwork, unsigned char *buffer, int len, int timeout_ms)
{
    uintptr_t       fd = pNetwork->handle;
    os_tick_t       xTicksToWait = os_tick_from_ms(timeout_ms); /* convert milliseconds to ticks. */ 
    Timer           xTimeOut = {0, 0, NULL};
    int             recvLen = 0;
    int             ret;
    struct timeval  tv;
    fd_set          sets;
		
    tv.tv_sec  = 0;
    tv.tv_usec = timeout_ms * 1000;

    TimerInit(&xTimeOut);
    TimerSetTimeOutState(&xTimeOut, xTicksToWait); /* Record the time at which this function was entered. */ 

    do
    {
        xTicksToWait = xTimeOut.xTicksToWait;
        tv.tv_sec = 0;
        tv.tv_usec = xTicksToWait * (1000 / OS_TICK_PER_SECOND) * 1000;
			
        FD_ZERO(&sets);
        FD_SET(fd, &sets);
			
        ret = select(fd + 1, &sets, NULL, NULL, &tv);
        if (ret > 0)
        {
            ret = recv(fd, buffer + recvLen, len - recvLen, 0);
            if (ret > 0)
            {
                recvLen += ret;
            }
            else if (0 == ret)
            {
                LOG_E(MQ_NET_TAG, "connection close");
                recvLen = -1;
                break;
            }
            else
            {
                if (EINTR == errno)
                {
                    LOG_E(MQ_NET_TAG, "EINTR be caught");
                    continue;
                }
                LOG_E(MQ_NET_TAG, "recv fail");
                recvLen = -1;
                break;
            }
        }
        else if (0 == ret)
        {
            break;
        }
        else
        {
            LOG_E(MQ_NET_TAG, "select-recv fail");
            recvLen = -1;
            break;
        }
    } while ((recvLen < len) && (TimerCheckForTimeOut(&xTimeOut) == FALSE));

    TimerRelease(&xTimeOut);

    return recvLen;
}

static int mqtt_write_tcp(Network *pNetwork, unsigned char *buffer, int len, int timeout_ms)
{  
    uintptr_t       fd = pNetwork->handle;
    os_tick_t       xTicksToWait = os_tick_from_ms(timeout_ms); /* convert milliseconds to ticks. */
    Timer           xTimeOut = {0, 0, NULL};
    int             sentLen = 0;
    int             ret;
    struct timeval  tv;
    fd_set          sets;
		
    ret        = 1;
    tv.tv_sec  = 0;
    tv.tv_usec = timeout_ms * 1000;
		
    TimerInit(&xTimeOut);
    TimerSetTimeOutState(&xTimeOut, xTicksToWait); /* Record the time at which this function was entered. */ 

    do	
    {
        if (0 != xTicksToWait)
        {
            xTicksToWait = xTimeOut.xTicksToWait;
            tv.tv_sec = 0;
            tv.tv_usec = xTicksToWait * (1000 / OS_TICK_PER_SECOND) * 1000;
            FD_ZERO(&sets);
            FD_SET(fd, &sets);

            tv.tv_sec  = xTicksToWait / 1000;
            tv.tv_usec = (xTicksToWait % 1000) * 1000;

            ret = select(fd + 1, NULL, &sets, NULL, &tv);
            if (ret > 0)
            {
                if (0 == FD_ISSET(fd, &sets))
                {
                    LOG_E(MQ_NET_TAG, "Should NOT arrive");
                    /* If timeout in next loop, it will not sent any data */
                    ret = 0;
                    continue;
                }
            }
            else if (0 == ret)
            {
                LOG_E(MQ_NET_TAG, "select-write timeout %d", timeout_ms);
                sentLen = -1;
                break;
            }
            else
            {
                if (EINTR == errno)
                {
                    LOG_E(MQ_NET_TAG, "EINTR be caught");
                    continue;
                }

                LOG_E(MQ_NET_TAG, "select-write fail");
                sentLen = -1;
                break;
            }
        }

        if (ret > 0)
        {
            ret = send(fd, buffer + sentLen, len - sentLen, 0);
            if (ret > 0)
            {
                sentLen += ret;
            }
            else if (0 == ret)
            {
                LOG_E(MQ_NET_TAG, "No data be sent");
            }
            else
            {
                if (EINTR == errno)
                {
                    LOG_E(MQ_NET_TAG, "EINTR be caught");
                    continue;
                }

                LOG_E(MQ_NET_TAG, "send fail");
                sentLen = -1;
                break;
            }
        }
    }while ((sentLen < len) && (TimerCheckForTimeOut(&xTimeOut) == FALSE));
    TimerRelease(&xTimeOut);

    return sentLen;
}

#endif

static int MQTT_net_read(Network *pNetwork, unsigned char *buffer, int len, int timeout_ms)
{
    int ret = 0;
#ifdef MQTT_USING_TLS
    #if defined(MQTT_USING_TLS_MBEDTLS)
        if (NULL != pNetwork->ca_crt)
        {
            ret = mqtt_read_ssl(pNetwork, buffer, len, timeout_ms);
        }
    #elif defined(MQTT_USING_TLS_ONETLS)
        if (NULL != pNetwork->psk_identity && NULL != pNetwork->psk_key)
        {
            ret = mqtt_read_ssl(pNetwork, buffer, len, timeout_ms);
        }
    #endif
#else
    if (NULL == pNetwork->ca_crt)
    {
        ret = mqtt_read_tcp(pNetwork, buffer, len, timeout_ms);
    }
#endif
    else
    {
        ret = -1;
        LOG_E(MQ_NET_TAG, "no method match!");
    }

    return ret;
}

static int MQTT_net_write(Network *pNetwork, unsigned char *buffer, int len, int timeout_ms)
{
    int ret = 0;
#ifdef MQTT_USING_TLS
    #if defined(MQTT_USING_TLS_MBEDTLS)
        if (NULL != pNetwork->ca_crt)
        {
            ret = mqtt_write_ssl(pNetwork, buffer, len, timeout_ms);
        }
    #elif defined(MQTT_USING_TLS_ONETLS)
        if (NULL != pNetwork->psk_identity && NULL != pNetwork->psk_key)
        {
            ret = mqtt_write_ssl(pNetwork, buffer, len, timeout_ms);
        }
    #endif        
#else
    if (NULL == pNetwork->ca_crt)
    {
        ret = mqtt_write_tcp(pNetwork, buffer, len, timeout_ms);
    }
#endif
    else
    {
        ret = -1;
        LOG_E(MQ_NET_TAG, "no method match!");
    }

    return ret;
}

#if defined(MQTT_USING_TLS_ONETLS)
/**
 ***********************************************************************************************************************
 * @brief           This function initialize mqtt network.
 *
 * @param[in]       pNetwork: a pointer to the struct Network.
 * @param[in]       host: a pointer to the url or ip.
 * @param[in]       port: port.
 * @param[in]       psk_identity: a pointer to psk identity.
 * @param[in]       psk_identity_len: psk identity length
 * @param[in]       psk_key: a pointer to the psk key
 * @param[in]       psk_key_len: psk identity length
 *
 * @return          Return status.
 * @retval          0           success.
 * @retval          -1          failed.
 ***********************************************************************************************************************
 */
int MQTTNetworkInit(Network *pNetwork, const char *host, uint16_t port, const uint8_t *psk_identity, uint16_t psk_identity_len, 
                    const uint8_t *psk_key, uint16_t psk_key_len)
{
    if (!pNetwork || !host || !psk_identity || !psk_key)
    {
        LOG_E(MQ_NET_TAG, "parameter error! pNetwork=%p, host = %p", pNetwork, host);
        return -1;
    }
    pNetwork->pHostAddress = host;
    pNetwork->port = port;
    
    pNetwork->psk_identity = psk_identity;
    pNetwork->psk_identity_len = psk_identity_len;
    pNetwork->psk_key_len = psk_key_len;
    pNetwork->psk_key = psk_key;

    // pNetwork->ca_crt = ca_crt;
    // if (NULL == ca_crt)
    // {
    //     pNetwork->ca_crt_len = 0;
    // }
    // else
    // {
    //     pNetwork->ca_crt_len = strlen(ca_crt);
    // }

    pNetwork->handle = 0;
    pNetwork->mqttread = MQTT_net_read;
    pNetwork->mqttwrite = MQTT_net_write;
    pNetwork->disconnect = MQTT_net_disconnect;
    pNetwork->connect = MQTT_net_connect;

    return 0;
}

#else
/**
 ***********************************************************************************************************************
 * @brief           This function initialize mqtt network.
 *
 * @param[in]       pNetwork    struct Network pointer.
 * @param[in]       host        url or ip.
 * @param[in]       port        port.
 * @param[in]       ca_crt      certificate.
 *
 * @return          Return status.
 * @retval          0           success.
 * @retval          -1          failed.
 ***********************************************************************************************************************
 */
int MQTTNetworkInit(Network *pNetwork, const char *host, uint16_t port, const char *ca_crt)
{
    if (!pNetwork || !host)
    {
        LOG_E(MQ_NET_TAG, "parameter error! pNetwork=%p, host = %p", pNetwork, host);
        return -1;
    }
    pNetwork->pHostAddress = host;
    pNetwork->port = port;
    pNetwork->ca_crt = ca_crt;

    if (NULL == ca_crt)
    {
        pNetwork->ca_crt_len = 0;
    }
    else
    {
        pNetwork->ca_crt_len = strlen(ca_crt);
    }

    pNetwork->handle = 0;
    pNetwork->mqttread = MQTT_net_read;
    pNetwork->mqttwrite = MQTT_net_write;
    pNetwork->disconnect = MQTT_net_disconnect;
    pNetwork->connect = MQTT_net_connect;

    return 0;
}

#endif

/**
 ***********************************************************************************************************************
 * @brief           This function connect mqtt network.
 *
 * @param[in]       pNetwork    struct Network pointer.
 *
 * @return          Return status.
 * @retval          0           success.
 * @retval          -1          failed.
 ***********************************************************************************************************************
 */
int MQTTNetworkConnect(Network *pNetwork)
{
    if (NULL == pNetwork)
        return -1;

    return MQTT_net_connect(pNetwork);
}

/**
 ***********************************************************************************************************************
 * @brief           This function disconnect mqtt network.
 *
 * @param[in]       pNetwork    struct Network pointer.
 *
 * @return          void
 ***********************************************************************************************************************
 */
void MQTTNetworkDisconnect(Network *pNetwork)
{
    if (NULL == pNetwork)
        return;

    MQTT_net_disconnect(pNetwork);
}
