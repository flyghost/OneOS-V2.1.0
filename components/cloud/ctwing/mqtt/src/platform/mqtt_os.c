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
 * @file        mqtt_os.c
 * 
 * @brief       Implementation of the functions in mqtt_os.h for OneOS systems. 
 * 
 * @details     
 * 
 * @revision
 * Date         Author          Notes
 * 2020-12-10   OneOs Team      First Version
 ***********************************************************************************************************************
 */

#include "mqtt_os.h"
#include "ctiot_log.h"
#ifdef CTIOT_MQTT_USING_TLS
#include "mbedtls/error.h"
#include "ctiot_tls_mbedtls.h"
#endif

#define U32_DIFF(a, b) (((a) >= (b)) ? ((a) - (b)) : (((a) + ((b) ^ 0xFFFFFFFF) + 1)))

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

void ThreadDestroy(Thread thread)
{   
    if(thread)
    {
        os_task_destroy(thread);
    }

}
#endif

void MutexInit(Mutex *mutex)
{
    os_mutex_init(mutex, "mqtt_lock", OS_FALSE);
}

void MutexDeInit(Mutex *mutex)
{
    os_mutex_deinit(mutex);
}

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

void TimerCallback(void *parameter)
{
    /*Becareful: do not include any debug printf code.*/
}

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

void TimerCountdownMS(Timer *timer, unsigned int timeout_ms)
{
    os_tick_t xTicksToWait;

    xTicksToWait = os_tick_from_ms(timeout_ms); /* convert milliseconds to ticks */
    TimerSetTimeOutState(timer, xTicksToWait);  /* Record the time at which this function was entered. */
}

void TimerCountdown(Timer *timer, unsigned int timeout)
{
    TimerCountdownMS(timer, timeout * 1000);
}

int TimerLeftMS(Timer *timer)
{
    return (timer->xTicksToWait * (1000 / OS_TICK_PER_SECOND));
}

char TimerIsExpired(Timer *timer)
{
    return TimerCheckForTimeOut(timer);
}

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

//#define	timeradd(a, b, result)						      \
//  do {									      \
//    (result)->tv_sec = (a)->tv_sec + (b)->tv_sec;			      \
//    (result)->tv_usec = (a)->tv_usec + (b)->tv_usec;			      \
//    if ((result)->tv_usec >= 1000000)					      \
//      {									      \
//	++(result)->tv_sec;						      \
//	(result)->tv_usec -= 1000000;					      \
//      }									      \
//  } while (0)
//#define	timersub(a, b, result)						      \
//  do {									      \
//    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;			      \
//    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec;			      \
//    if ((result)->tv_usec < 0) {					      \
//      --(result)->tv_sec;						      \
//      (result)->tv_usec += 1000000;					      \
//    }									      \
//  } while (0)

//void TimerInit(Timer* timer)
//{
//	timer->end_time = (struct timeval){0, 0};
//}

//char TimerIsExpired(Timer* timer)
//{
//	struct timeval now, res;
//	gettimeofday(&now, NULL);
//	timersub(&timer->end_time, &now, &res);
//	return res.tv_sec < 0 || (res.tv_sec == 0 && res.tv_usec <= 0);
//}


//void TimerCountdownMS(Timer* timer, unsigned int timeout)
//{
//	struct timeval now;
//	gettimeofday(&now, NULL);
//	struct timeval interval = {timeout / 1000, (timeout % 1000) * 1000};
//	timeradd(&now, &interval, &timer->end_time);
//}


//void TimerCountdown(Timer* timer, unsigned int timeout)
//{
//	struct timeval now;
//	gettimeofday(&now, NULL);
//	struct timeval interval = {timeout, 0};
//	timeradd(&now, &interval, &timer->end_time);
//}


//int TimerLeftMS(Timer* timer)
//{
//	struct timeval now, res;
//	gettimeofday(&now, NULL);
//	timersub(&timer->end_time, &now, &res);
//	//printf("left %d ms\n", (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000);
//	return (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000;
//}

/*
int linux_read(Network* n, unsigned char* buffer, int len, int timeout_ms,char* file,unsigned long line)
{
	struct timeval interval = {timeout_ms / 1000, (timeout_ms % 1000) * 1000};
	if (interval.tv_sec < 0 || (interval.tv_sec == 0 && interval.tv_usec <= 0))
	{
		interval.tv_sec = 0;
		interval.tv_usec = 100;
	}

	setsockopt(n->my_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&interval, sizeof(struct timeval));
	if(socket_connected(n->my_socket)==0)
	{
	    return -1;
	}
	int bytes = 0;
	while (bytes < len)
	{		
		int rc = recv(n->my_socket, &buffer[bytes], (size_t)(len - bytes), 0);
		if (rc < 0)
		{
			if (errno != EAGAIN && errno != EWOULDBLOCK)
			{
				CTIOT_LOG(LOG_ERR, "Read socket: rc<0, %u\n",errno);
                bytes = -1;
			}
			break;
		}
		else if (rc == 0)
		{
            //printf("Read socket: rc=0\n");
			bytes = 0;
			break;
		}
		else
		{
            bytes += rc;
		}
	}    
	return bytes;
}


int linux_write(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
	struct timeval tv;

	tv.tv_sec = 0;  *//* 30 Secs Timeout *//*
	tv.tv_usec = timeout_ms * 1000;  // Not init'ing this can cause strange errors
	//#ifdef ID_LEVEL_LOG_COMPILE
    //log_print_plain_text("linux_write:1\n");
	//#endif
	setsockopt(n->my_socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv,sizeof(struct timeval));
    //#ifdef ID_LEVEL_LOG_COMPILE
    //log_print_plain_text("linux_write:2\n");
	//#endif
	int rc=0;
	if(socket_connected(n->my_socket)!=0)
	{   
	    rc = write(n->my_socket, buffer, len);
	}
	//#ifdef ID_LEVEL_LOG_COMPILE
    //log_print_plain_text("linux_write:rc=%d\n",rc);
	//#endif
	return rc;
}*/


#ifdef CTIOT_MQTT_USING_TLS
static uintptr_t ctiot_ssl_establish(const char *host, uint16_t port, const char *ca_crt, uint32_t ca_crt_len)
{
    char        port_str[6] = {0};
    uintptr_t   tls_session_ptr = (uintptr_t)NULL;

    if (host == NULL || ca_crt == NULL)
    {
        CTIOT_LOG(LOG_ERR, "input params are NULL, abort");
        return (uintptr_t)NULL;
    }

    if (!strlen(host) || (strlen(host) < 8))
    {
        CTIOT_LOG(LOG_ERR, "invalid host: '%s'(len=%d), abort", host, (int)strlen(host));
        return (uintptr_t)NULL;
    }

    sprintf(port_str, "%u", port);
    if (0 != ctiot_tls_network_mbedtls_establish(&tls_session_ptr, host, port_str, ca_crt, NULL, NULL, NULL))
    {
        return (uintptr_t)NULL;
    }

    return (uintptr_t)(tls_session_ptr);
}

static int ctiot_connect_ssl(Network *pNetwork)
{
    if (NULL == pNetwork)
    {
        CTIOT_LOG(LOG_ERR, "network is null");
        return -1;
    }

    pNetwork->handle = (intptr_t)ctiot_ssl_establish(pNetwork->pHostAddress, 
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

static int ctiot_disconnect_ssl(Network *pNetwork)
{
    if (NULL == pNetwork)
    {
        CTIOT_LOG(LOG_ERR, "network is null");
        return -1;
    }

    ctiot_tls_network_mbedtls_close((ctiot_tls_session_t **)&(pNetwork->handle));
    pNetwork->handle = (uintptr_t)NULL; /* must have */
    CTIOT_LOG(LOG_INFO, "MbedTLS connection close success.");

    return 0;
}

#else
static int ctiot_connect_tcp(Network *pNetwork)
{
    int                 fd = 0;
    int                 retVal = -1;
    struct sockaddr_in  sAddr;
    struct hostent     *host_entry = NULL;
    long                socket_mode = 1; /* non-blocking */

    if (NULL == pNetwork)
    {
        CTIOT_LOG(LOG_ERR, "MQTT pNetwork is null");
        return -1;
    }

    if ((host_entry = gethostbyname(pNetwork->pHostAddress)) == NULL)
    {
        CTIOT_LOG(LOG_ERR, "dns parse error!");
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

static int ctiot_disconnect_tcp(Network *pNetwork)
{
    if ((uintptr_t)(-1) == pNetwork->handle)
    {
        CTIOT_LOG(LOG_ERR, "MQTT_Network->handle = -1");
        return -1;
    }
	closesocket(pNetwork->handle);
    pNetwork->handle = (uintptr_t)(-1);
    CTIOT_LOG(LOG_INFO, "TCP connection close success.");

    return 0;
}
#endif

static int ctiot_net_connect(Network *pNetwork)
{
    int ret = 0;
#ifdef CTIOT_MQTT_USING_TLS
    if (NULL != pNetwork->ca_crt)
    {
        ret = ctiot_connect_ssl(pNetwork);
    }
#else
    if (NULL == pNetwork->ca_crt)
    {
        ret = ctiot_connect_tcp(pNetwork);
    }
#endif
    else
    {
        ret = -1;
        CTIOT_LOG(LOG_ERR, "no method match!");
    }

    return ret;
}

static int ctiot_net_disconnect(Network *pNetwork)
{
    int ret = 0;
#ifdef CTIOT_MQTT_USING_TLS
    if (NULL != pNetwork->ca_crt)
    {
        ret = ctiot_disconnect_ssl(pNetwork);
    }
#else
    if (NULL == pNetwork->ca_crt)
    {
        ret = ctiot_disconnect_tcp(pNetwork);
    }
#endif
    else
    {
        ret = -1;
        CTIOT_LOG(LOG_ERR, "no method match!");
    }

    return ret;
}

#ifdef CTIOT_MQTT_USING_TLS
static int ctiot_read_ssl(Network *pNetwork, unsigned char *buffer, int len, int timeout_ms)
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

    ctiot_tls_session_t *session_ptr = (ctiot_tls_session_t *)pNetwork->handle;

    do
    {
        xTicksToWait = xTimeOut.xTicksToWait;
        xMsToWait = xTicksToWait * (1000 / OS_TICK_PER_SECOND);

        mbedtls_ssl_conf_read_timeout(&(session_ptr->config), xMsToWait);
        rc = ctiot_tls_network_mbedtls_read((ctiot_tls_session_t *)pNetwork->handle, 
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
                CTIOT_LOG(LOG_ERR, "ssl recv error: code = %d, err_str = '%s'", rc, err_str);
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
                CTIOT_LOG(LOG_ERR, "ssl recv error: code = %d, err_str = '%s'", rc, err_str);
                net_status = -1;
                recvLen = net_status;
                break;
            }
        }

    } while ((recvLen < len) && (TimerCheckForTimeOut(&xTimeOut) == FALSE));

    TimerRelease(&xTimeOut);

    return recvLen;
}

static int ctiot_write_ssl(Network *pNetwork, unsigned char *buffer, uint32_t len, uint32_t timeout_ms)
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
        rc = ctiot_tls_network_mbedtls_write((ctiot_tls_session_t *)pNetwork->handle, 
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
                CTIOT_LOG(LOG_ERR, "mbedtls_ssl_write returned -0x%x", -rc);
                sentLen = rc;
                break;
            }
        }
    } while ((sentLen < len) && (TimerCheckForTimeOut(&xTimeOut) == OS_FALSE));

    TimerRelease(&xTimeOut);

    return sentLen;
}

#else

static int ctiot_read_tcp(Network *pNetwork, unsigned char *buffer, int len, int timeout_ms)
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
                CTIOT_LOG(LOG_ERR, "connection close");
                recvLen = -1;
                break;
            }
            else
            {
                if (EINTR == errno)
                {
                    CTIOT_LOG(LOG_ERR, "EINTR be caught");
                    continue;
                }
                CTIOT_LOG(LOG_ERR, "recv fail");
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
            CTIOT_LOG(LOG_ERR, "select-recv fail");
            recvLen = -1;
            break;
        }
    } while ((recvLen < len) && (TimerCheckForTimeOut(&xTimeOut) == OS_FALSE));

    TimerRelease(&xTimeOut);

    return recvLen;
}

static int ctiot_write_tcp(Network *pNetwork, unsigned char *buffer, int len, int timeout_ms)
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
                    CTIOT_LOG(LOG_ERR, "Should NOT arrive");
                    /* If timeout in next loop, it will not sent any data */
                    ret = 0;
                    continue;
                }
            }
            else if (0 == ret)
            {
                CTIOT_LOG(LOG_ERR, "select-write timeout %d", timeout_ms);
                sentLen = -1;
                break;
            }
            else
            {
                if (EINTR == errno)
                {
                    CTIOT_LOG(LOG_ERR, "EINTR be caught");
                    continue;
                }

                CTIOT_LOG(LOG_ERR, "select-write fail");
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
                CTIOT_LOG(LOG_ERR, "No data be sent");
            }
            else
            {
                if (EINTR == errno)
                {
                    CTIOT_LOG(LOG_ERR, "EINTR be caught");
                    continue;
                }

                CTIOT_LOG(LOG_ERR, "send fail");
                sentLen = -1;
                break;
            }
        }
    }while ((sentLen < len) && (TimerCheckForTimeOut(&xTimeOut) == FALSE));
    TimerRelease(&xTimeOut);

    return sentLen;
}

#endif

static int ctiot_net_read(Network *pNetwork, unsigned char *buffer, int len, int timeout_ms)
{
    int ret = 0;
#ifdef CTIOT_MQTT_USING_TLS
    if (NULL != pNetwork->ca_crt)
    {
        ret = ctiot_read_ssl(pNetwork, buffer, len, timeout_ms);
    }
#else
    if (NULL == pNetwork->ca_crt)
    {
        ret = ctiot_read_tcp(pNetwork, buffer, len, timeout_ms);
    }
#endif
    else
    {
        ret = -1;
        CTIOT_LOG(LOG_ERR, "no method match!");
    }

    return ret;
}

static int ctiot_net_write(Network *pNetwork, unsigned char *buffer, int len, int timeout_ms)
{
    int ret = 0;
#ifdef CTIOT_MQTT_USING_TLS
    if (NULL != pNetwork->ca_crt)
    {
        ret = ctiot_write_ssl(pNetwork, buffer, len, timeout_ms);
    }
#else
    if (NULL == pNetwork->ca_crt)
    {
        ret = ctiot_write_tcp(pNetwork, buffer, len, timeout_ms);
    }
#endif
    else
    {
        ret = -1;
        CTIOT_LOG(LOG_ERR, "no method match!");
    }

    return ret;
}

int NetworkInit(Network *pNetwork, const char *host, uint16_t port, const char *ca_crt)
{
    if (!pNetwork || !host)
    {
        CTIOT_LOG(LOG_ERR, "parameter error! pNetwork=%p, host = %p", pNetwork, host);
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
    pNetwork->mqttread = ctiot_net_read;
    pNetwork->mqttwrite = ctiot_net_write;
    pNetwork->disconnect = ctiot_net_disconnect;
    pNetwork->connect = ctiot_net_connect;

    return 0;
}

int NetworkConnect(Network* n)
{
	if( NULL == n )
		return -1;
	
	return ctiot_net_connect(n);
}

void NetworkDisconnect(Network* n)
{
	if( NULL == n )
		return;
	
	ctiot_net_disconnect(n);
}
