/*******************************************************************************
 *
 * Copyright (c) 2015 Intel Corporation and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * The Eclipse Distribution License is available at
 *    http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    David Navarro, Intel Corporation - initial API and implementation
 *    Christian Renz - Please refer to git log
 *
 *******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/errno.h>
#include <oneos_config.h>
#ifdef CONFIG_LWM2M_SECURE
#include "lwm2mdebug.h"
#include "mbedconnection.h"
#include "commandline.h"
#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif
#include "mbedtls/net_sockets.h"
#include "mbedtls/debug.h"
//#include "mbedtls/net_sockets.h"
#include <sys/socket.h>

#include <sys/select.h>


#define COAP_PORT "5683"
#define COAPS_PORT "5684"
#define URI_LENGTH 256

#define MAX_PACKET_SIZE 1024

/* Currently, just IPV4 enabled */
#define LWIP_IPV4_ON 1

extern int sockaddr_cmp(struct sockaddr *x, struct sockaddr *y);

/********************* Security Obj Helpers **********************/
char * security_get_uri(lwm2m_object_t * obj, int instanceId, char * uriBuffer, int bufferSize){
    int size = 1;
    lwm2m_data_t * dataP = lwm2m_data_new(size);
    dataP->id = 0; // security server uri

    obj->readFunc(instanceId, &size, &dataP, obj);
	
    lwm2m_log(COMP_LOG_DEBUG, "length=%d,type=%d,data=%x", dataP->value.asBuffer.length,dataP->type,dataP);
    if (dataP != NULL &&
            dataP->type == LWM2M_TYPE_STRING &&
            dataP->value.asBuffer.length > 0)
    {
        if (bufferSize > dataP->value.asBuffer.length){
            memset(uriBuffer,0,dataP->value.asBuffer.length+1);
            strncpy(uriBuffer,(char *)dataP->value.asBuffer.buffer,dataP->value.asBuffer.length);
            lwm2m_data_free(size, dataP);
            return uriBuffer;
        }
    }
    lwm2m_data_free(size, dataP);
    return NULL;
}

int64_t security_get_mode(lwm2m_object_t * obj, int instanceId){
    int64_t mode;
    int size = 1;
    lwm2m_data_t * dataP = lwm2m_data_new(size);
    dataP->id = 2; // security mode

    obj->readFunc(instanceId, &size, &dataP, obj);
    if (0 != lwm2m_data_decode_int(dataP,&mode))
    {
        lwm2m_data_free(size, dataP);
        return mode;
    }

    lwm2m_data_free(size, dataP);
    lwm2m_log(COMP_LOG_ERROR, "Unable to get security mode : use not secure mode\n");
    return LWM2M_SECURITY_MODE_NONE;
}

char * security_get_public_id(lwm2m_object_t * obj, int instanceId, int * length){
    int size = 1;
    lwm2m_data_t * dataP = lwm2m_data_new(size);
    dataP->id = 3; // public key or id

    obj->readFunc(instanceId, &size, &dataP, obj);
    if (dataP != NULL &&
        dataP->type == LWM2M_TYPE_OPAQUE)
    {
        char * buff;

        buff = (char*)lwm2m_malloc(dataP->value.asBuffer.length);
        if (buff != 0)
        {
            memcpy(buff, dataP->value.asBuffer.buffer, dataP->value.asBuffer.length);
            *length = dataP->value.asBuffer.length;
        }
        lwm2m_data_free(size, dataP);

        return buff;
    } else {
        return NULL;
    }
}

static char * security_get_secret_key(lwm2m_object_t * obj, int instanceId, int * length){
    int size = 1;
    lwm2m_data_t * dataP = lwm2m_data_new(size);
    dataP->id = 5; // secret key
    lwm2m_log(COMP_LOG_DEBUG, "security_get_secret_key\n");
    obj->readFunc(instanceId, &size, &dataP, obj);
    if (dataP != NULL &&
        dataP->type == LWM2M_TYPE_OPAQUE)
    {
        char * buff;
        buff = (char*)lwm2m_malloc(dataP->value.asBuffer.length);
        if (buff != 0)
        {
            memcpy(buff, dataP->value.asBuffer.buffer, dataP->value.asBuffer.length);
			*length = dataP->value.asBuffer.length;
        }
        lwm2m_data_free(size, dataP);
        return buff;
    } else {
        return NULL;
    }
}

/********************* Security Obj Helpers Ends **********************/

int send_data(dtls_connection_t *connP,
                    uint8_t * buffer,
                    size_t length)
{
    int nbSent;
    size_t offset;

#ifdef WITH_LOGS
    char s[INET6_ADDRSTRLEN];
    in_port_t port;

    s[0] = 0;

    if (AF_INET == connP->addr.sin6_family)
    {
        struct sockaddr_in *saddr = (struct sockaddr_in *)&connP->addr;
        inet_ntop(saddr->sin_family, &saddr->sin_addr, s, INET6_ADDRSTRLEN);
        port = saddr->sin_port;
    }
    else if (AF_INET6 == connP->addr.sin6_family)
    {
        struct sockaddr_in6 *saddr = (struct sockaddr_in6 *)&connP->addr;
        inet_ntop(saddr->sin6_family, &saddr->sin6_addr, s, INET6_ADDRSTRLEN);
        port = saddr->sin6_port;
    }

    lwm2m_log(COMP_LOG_DEBUG, "Sending %d bytes to [%s]:%hu\n", length, s, ntohs(port));

#endif

    offset = 0;
    while (offset != length)
    {
        nbSent = sendto(connP->sock, buffer + offset, length - offset, 0, (struct sockaddr *)&(connP->addr), connP->addrLen);
        if (nbSent == -1) return -1;
        offset += nbSent;
    }
    connP->lastSend = lwm2m_gettime();
    return 0;

}

/*
 * Write at most 'len' characters
 */
int mbedtls_lwm2m_send( void *connP, const unsigned char *buf, size_t len )
{
    dtls_connection_t* ctx = (dtls_connection_t *) connP;
    int fd = ctx->sock;
    int nbSent = -1;
    size_t offset;

    if( fd < 0 )
        return( MBEDTLS_ERR_NET_INVALID_CONTEXT );

#ifdef WITH_LOGS
#if LWIP_IPV4_ON
    if (AF_INET == ctx->addr.sin_family)
    {
        char s[INET_ADDRSTRLEN];
        struct sockaddr_in *saddr = (struct sockaddr_in *)&ctx->addr;
        inet_ntop(saddr->sin_family, &saddr->sin_addr, s, INET_ADDRSTRLEN);
    }
#endif
#if LWIP_IPV6_ON
    if (AF_INET6 == ctx->addr6.sin6_family)
    {
        char s[INET6_ADDRSTRLEN];
        struct sockaddr_in6 *saddr = (struct sockaddr_in6 *)&ctx->addr;
        inet_ntop(saddr->sin6_family, &saddr->sin6_addr, s, INET6_ADDRSTRLEN);
    }
#endif
    lwm2m_log(COMP_LOG_DEBUG, "Sending %d bytes\n", len);
#endif

    offset = 0;
    while (offset != len)
    {
        nbSent = sendto(fd, buf + offset, len - offset, 0, (struct sockaddr *)&(ctx->addr), ctx->addrLen);
        if (nbSent == -1)
        {
            if(errno != EINTR && errno != EWOULDBLOCK && errno != EINPROGRESS && errno != EAGAIN && errno != EALREADY)
            {
                continue;
            }
            else
            {
                return -1;
            }
        }
        offset += nbSent;
    }
    lwm2m_log(COMP_LOG_DEBUG, "return len = %d\n", len);
    return nbSent;
}

/*
 * Read at most 'len' characters
 */
int mbedtls_lwm2m_recv( void *ctx, unsigned char *buffer, size_t len )
{
    int fd = ((dtls_connection_t *) ctx)->sock;
    struct timeval tv;
    int result;
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    fd_set readfds;

    if( fd < 0 )
    {
        return( MBEDTLS_ERR_NET_INVALID_CONTEXT );
    }

    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);

    /*
     * This part will set up an interruption until an event happen on SDTIN or the socket until "tv" timed out (set
     * with the precedent function)
     */
    result = select(FD_SETSIZE, &readfds, NULL, NULL, &tv);

    if (result < 0)
    {
        lwm2m_log(COMP_LOG_ERROR, "Error in select(): %d\n", result);
        return result;
    }
    else if (result > 0)
    {
        int numBytes;
	lwm2m_log(COMP_LOG_DEBUG, "select read event happend\n");

        /*
         * If an event happens on the socket
         */
        if (FD_ISSET(fd, &readfds))
        {
            struct sockaddr_storage addr;
            socklen_t addrLen;
            addrLen = sizeof(addr);

            /*
             * We retrieve the data received
             */
            numBytes = recvfrom(fd, buffer, MAX_PACKET_SIZE, 0, (struct sockaddr *)&addr, &addrLen);

            if (0 > numBytes)
            {
                lwm2m_log(COMP_LOG_DEBUG, "Error in recvfrom()");
            }
            else if (0 < numBytes)
            {
#ifdef WITH_LOGS
#if LWIP_IPV4_ON
                if (AF_INET == addr.ss_family)
                {
                    char s[INET_ADDRSTRLEN];
                    in_port_t port = 0;
                    struct sockaddr_in *saddr = (struct sockaddr_in *)&addr;
                    inet_ntop(saddr->sin_family, &saddr->sin_addr, s, INET_ADDRSTRLEN);
                    port = saddr->sin_port;
                    lwm2m_log(COMP_LOG_DEBUG, "%d bytes received from [%s]:%hu\n", numBytes, s, ntohs(port));
                }
#endif
#if LWIP_IPV6_ON
                if (AF_INET6 == addr.ss_family)
                {
                    char s[INET6_ADDRSTRLEN];
                    in_port_t port = 0;
                    struct sockaddr_in6 *saddr = (struct sockaddr_in6 *)&addr;
                    inet_ntop(saddr->sin6_family, &saddr->sin6_addr, s, INET6_ADDRSTRLEN);
                    port = saddr->sin6_port;
                    lwm2m_log(COMP_LOG_DEBUG, "%d bytes received from [%s]:%hu\n", numBytes, s, ntohs(port));
                }
#endif
#endif
            }

            return numBytes;
	}

        lwm2m_log(COMP_LOG_DEBUG, "no read event happend on this socket\n");
        return -1;
    }else{
    	lwm2m_log(COMP_LOG_ERROR, "timeout !!!\n");
    	return -1;
    }
}

static void lwm2m_debug( void *ctx, int level,
                  const char *file, int line,
                  const char *param )
{
    ((void) level);
    lwm2m_log(COMP_LOG_DEBUG, "%s\n",param);

}
				  
#ifndef MBEDTLS_ENTROPY_C
static int ssl_random(void *prng, unsigned char *output, size_t output_len)
{
    uint32_t rnglen = output_len;
    uint8_t rngoffset = 0;

    while (rnglen > 0) {
        *(output + rngoffset) = (uint8_t)rand();
        rngoffset++;
        rnglen--;
    }

    return 0;
}
#endif

int get_dtls_context(dtls_connection_t * connList) {
    int ret;
#ifdef MBEDTLS_ENTROPY_C
    const char *pers ="lwm2mclient";
#endif
#if defined(MBEDTLS_CCM_C) && defined(MBEDTLS_AES_C)
    int force_ciphersuite[2];
#endif /* MBEDTLS_CCM_C &&  MBEDTLS_AES_C */

    lwm2m_log(COMP_LOG_DEBUG, "Entering get_dtls_context\n");

    connList->server_fd = lwm2m_malloc(sizeof(mbedtls_net_context));
    mbedtls_net_init(connList->server_fd);

    connList->ssl = lwm2m_malloc(sizeof(mbedtls_ssl_context));
    mbedtls_ssl_init(connList->ssl);
    connList->conf = lwm2m_malloc(sizeof(mbedtls_ssl_config));
    mbedtls_ssl_config_init(connList->conf);
    connList->cacert = lwm2m_malloc(sizeof(mbedtls_x509_crt));
    mbedtls_x509_crt_init(connList->cacert);
#ifdef MBEDTLS_ENTROPY_C
    connList->ctr_drbg = lwm2m_malloc(sizeof(mbedtls_ctr_drbg_context));
    mbedtls_ctr_drbg_init(connList->ctr_drbg);
    connList->entropy = lwm2m_malloc(sizeof(mbedtls_entropy_context));
    mbedtls_entropy_init(connList->entropy);
    if(( ret = mbedtls_ctr_drbg_seed( connList->ctr_drbg, mbedtls_entropy_func, connList->entropy,
        (const unsigned char *) pers, strlen( pers ) ) ) != 0 )
    {
        lwm2m_log(COMP_LOG_ERROR, "mbedtls_ctr_drbg_seed failed...,ret=%d\n",ret);
        return ret;
    }
#endif

    connList->server_fd->fd = connList->sock;
    lwm2m_log(COMP_LOG_DEBUG, "mbedtls_use the sock=%d\n",connList->sock);
    if( ( ret = mbedtls_ssl_config_defaults( connList->conf,
            MBEDTLS_SSL_IS_CLIENT,
            MBEDTLS_SSL_TRANSPORT_DATAGRAM,
            MBEDTLS_SSL_PRESET_DEFAULT ) ) != 0 )
    {
        lwm2m_log(COMP_LOG_ERROR, "mbedtls_ssl_config_defaults failed ret = %d\n",ret);
        return ret;
    }

#if defined(MBEDTLS_CCM_C) && defined(MBEDTLS_AES_C)
    force_ciphersuite[0] = mbedtls_ssl_get_ciphersuite_id( "TLS-PSK-WITH-AES-128-CCM-8" );
    force_ciphersuite[1] = 0;
    mbedtls_ssl_conf_ciphersuites( connList->conf, force_ciphersuite );
#endif /* MBEDTLS_CCM_C &&  MBEDTLS_AES_C */

#if defined(MBEDTLS_NET_C)
    ret = mbedtls_net_set_block( connList->server_fd );
#endif
    mbedtls_timing_delay_context *timer = lwm2m_malloc(sizeof (mbedtls_timing_delay_context));
    mbedtls_ssl_set_timer_cb( connList->ssl, timer, mbedtls_timing_set_delay,
                                        mbedtls_timing_get_delay );

    int length = 0;
    int id_len = 0;
#if defined(MBEDTLS_KEY_EXCHANGE__SOME__PSK_ENABLED)
    char* psk = security_get_secret_key(connList->securityObj,connList->securityInstId,&length);
    char* psk_id = security_get_public_id(connList->securityObj,connList->securityInstId,&id_len);

    if( ( ret = mbedtls_ssl_conf_psk( connList->conf, (const unsigned char*)psk, length,
                         (const unsigned char*)psk_id,
                         id_len ) ) != 0 )
    {
        lwm2m_log(COMP_LOG_ERROR, " failed! mbedtls_ssl_conf_psk returned %d\n", ret);
        return ret;
    }
#endif
    /* OPTIONAL is not optimal for security,
    * but makes interop easier in this simplified example */
    mbedtls_ssl_conf_authmode( connList->conf, MBEDTLS_SSL_VERIFY_REQUIRED);
    mbedtls_ssl_conf_ca_chain( connList->conf, connList->cacert, NULL );
#ifdef MBEDTLS_ENTROPY_C
    mbedtls_ssl_conf_rng( connList->conf, mbedtls_ctr_drbg_random, connList->ctr_drbg );
#else
    mbedtls_ssl_conf_rng( connList->conf, ssl_random, NULL );
#endif
    mbedtls_ssl_conf_dbg( connList->conf, lwm2m_debug, NULL );
#ifdef MBEDTLS_DEBUG_C
    mbedtls_debug_set_threshold(3);
#endif
    if( ( ret = mbedtls_ssl_setup( connList->ssl, connList->conf ) ) != 0 )
    {
        lwm2m_log(COMP_LOG_ERROR, "mbedtls_ssl_setup failed ret = %d\n",ret);
        return ret;
    }
    mbedtls_ssl_set_bio( connList->ssl, connList, mbedtls_lwm2m_send, mbedtls_lwm2m_recv, NULL );
    /*
    * 4. Handshake
    */
    while( ( ret = mbedtls_ssl_handshake( connList->ssl ) ) != 0 )
    {
    	if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
    	{
    		lwm2m_log(COMP_LOG_ERROR, " failed ! mbedtls_ssl_handshake returned %x\n", -ret );
    		return ret;
    	}
    }

    lwm2m_log(COMP_LOG_DEBUG, " ok\n");
    return 0;

}

int get_port(struct sockaddr *x)
{
#if LWIP_IPV4_ON
   if (x->sa_family == AF_INET)
   {
       return ((struct sockaddr_in *)x)->sin_port;
   }
#endif
#if LWIP_IPV6_ON
   if (x->sa_family == AF_INET6) {
       return ((struct sockaddr_in6 *)x)->sin6_port;
   }
#endif
   lwm2m_log(COMP_LOG_ERROR, "err family(%d) set\n", x->sa_family);
   return  -1;

}

int sockaddr_cmp(struct sockaddr *x, struct sockaddr *y)
{
    int portX = get_port(x);
    int portY = get_port(y);

    // if the port is invalid of different
    if (portX == -1 || portX != portY)
    {
        return 0;
    }

    // IPV4?
    if (x->sa_family == AF_INET)
    {
        // is V4?
        if (y->sa_family == AF_INET)
        {
            // compare V4 with V4
            return ((struct sockaddr_in *)x)->sin_addr.s_addr == ((struct sockaddr_in *)y)->sin_addr.s_addr;
            // is V6 mapped V4?
        }
#if LWIP_IPV6_ON
        else if (IN6_IS_ADDR_V4MAPPED(&((struct sockaddr_in6 *)y)->sin6_addr)) {
            struct in6_addr* addr6 = &((struct sockaddr_in6 *)y)->sin6_addr;
            uint32_t y6to4 = addr6->s6_addr[15] << 24 | addr6->s6_addr[14] << 16 | addr6->s6_addr[13] << 8 | addr6->s6_addr[12];
            return y6to4 == ((struct sockaddr_in *)x)->sin_addr.s_addr;
        }
#endif
        else {
            return 0;
        }
    }
#if LWIP_IPV6_ON
    else if (x->sa_family == AF_INET6 && y->sa_family == AF_INET6) {
        // IPV6 with IPV6 compare
        return memcmp(((struct sockaddr_in6 *)x)->sin6_addr.s6_addr, ((struct sockaddr_in6 *)y)->sin6_addr.s6_addr, 16) == 0;
    } else
#endif
    {
        // unknown address type
        lwm2m_log(COMP_LOG_ERROR, "non IPV4 or IPV6 address\n");
        return 0;
    }
}

int create_socket(const char * portStr, int ai_family)
{
    lwm2m_log(COMP_LOG_DEBUG, "Entering create_socket\n");

    int s = -1;
    struct addrinfo hints;
    struct addrinfo *res;
    struct addrinfo *p;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = ai_family;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = IPPROTO_UDP;

    if (0 != getaddrinfo(NULL, portStr, &hints, &res))
    {
        lwm2m_log(COMP_LOG_ERROR, "getaddrinfo failed\n");
        return -1;
    }

    for(p = res ; p != NULL && s == -1 ; p = p->ai_next)
    {
        s = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (s >= 0)
        {
            int flag = 1;
            if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0)
            {
                closesocket(s);
                s = -1;
                lwm2m_log(COMP_LOG_ERROR, "setsockopt SO_REUSEADDR failed\n");
            }
            else if (-1 == bind(s, p->ai_addr, p->ai_addrlen))
            {
                closesocket(s);
                s = -1;
                lwm2m_log(COMP_LOG_ERROR, "bind failed\n");
            }
        }
    }

    freeaddrinfo(res);

    return s;
}

dtls_connection_t * connection_new_incoming(dtls_connection_t * connList,
                                       int sock,
                                       const struct sockaddr * addr,
                                       size_t addrLen)
{
    dtls_connection_t * connP;

    connP = (dtls_connection_t *)lwm2m_malloc(sizeof(dtls_connection_t));
    if (connP != NULL)
    {
        connP->sock = sock;
        memcpy(&(connP->addr), addr, addrLen);
        connP->addrLen = addrLen;
        connP->next = connList;
        lwm2m_log(COMP_LOG_DEBUG, "new_incoming\n");
    }

    return connP;
}

dtls_connection_t * connection_find(dtls_connection_t * connList,
                               const struct sockaddr_storage * addr,
                               size_t addrLen)
{
    dtls_connection_t * connP;

    connP = connList;
    while (connP != NULL)
    {
       if (sockaddr_cmp((struct sockaddr*) (&connP->addr),(struct sockaddr*) addr))
       {
            return connP;
       }

       connP = connP->next;
    }

    return connP;
}

dtls_connection_t * connection_create(dtls_connection_t * connList,
                                 int sock,
                                 lwm2m_object_t * securityObj,
                                 int instanceId,
                                 lwm2m_context_t * lwm2mH,
                                 int addressFamily)
{
    struct addrinfo hints;
    struct addrinfo *servinfo = NULL;
    struct addrinfo *p;
    int s;
    struct sockaddr *sa = NULL;
    socklen_t sl = 0;
    dtls_connection_t * connP = NULL;
    char uriBuf[URI_LENGTH];
    char * uri;
    char * host;
    char * port;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = addressFamily;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    uri = security_get_uri(securityObj, instanceId, uriBuf, URI_LENGTH);
	
    lwm2m_log(COMP_LOG_DEBUG, "uri=%s", uri);
    if (uri == NULL) return NULL;

    // parse uri in the form "coaps://[host]:[port]"
    char * defaultport;
    if (0 == strncmp(uri, "coaps://", strlen("coaps://")))
    {
        host = uri+strlen("coaps://");
        defaultport = COAPS_PORT;
    }
    else if (0 == strncmp(uri, "coap://", strlen("coap://")))
    {
        host = uri+strlen("coap://");
        defaultport = COAP_PORT;
    }
    else
    {
        return NULL;
    }
    port = strrchr(host, ':');
    if (port == NULL)
    {
        port = defaultport;
    }
    else
    {
        // remove brackets
        if (host[0] == '[')
        {
            host++;
            if (*(port - 1) == ']')
            {
                *(port - 1) = 0;
            }
            else
            {
                return NULL;
            }
        }
        // split strings
        *port = 0;
        port++;
    }
    lwm2m_log(COMP_LOG_DEBUG, "host=%s,port=%s,ai_family=%d", host, port, hints.ai_family);
	//int result = getaddrinfo(host, port, &hints, &servinfo);
    if (0 != getaddrinfo(host, port, &hints, &servinfo))
   	{
		lwm2m_log(COMP_LOG_DEBUG, "servinfo=%x", servinfo);
		return NULL;
    }
    // we test the various addresses
    s = -1;
    for(p = servinfo ; p != NULL && s == -1 ; p = p->ai_next)
    {
        s = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (s >= 0)
        {
            sa = p->ai_addr;
            sl = p->ai_addrlen;
            if (-1 == connect(s, p->ai_addr, p->ai_addrlen))
            {
                lwm2m_log(COMP_LOG_ERROR, "failed\n");
                close(s);
                s = -1;
            }
        }
    }
    if (s >= 0)
    {
        connP = connection_new_incoming(connList, sock, sa, sl);
        closesocket(s);
        if (connP != NULL)
        {
            lwm2m_log(COMP_LOG_DEBUG, " connP != NULL");
            connP->next = connList;
            connP->securityObj = securityObj;
            connP->securityInstId = instanceId;
            connP->lwm2mH = lwm2mH;

            if (security_get_mode(connP->securityObj,connP->securityInstId)
                     != LWM2M_SECURITY_MODE_NONE)
            {
                connP->issecure = 1;
                int ret = get_dtls_context(connP);
                if (ret != 0)
                {
                    if (NULL != servinfo) freeaddrinfo(servinfo);
                    return NULL;
                }
            }
            else
            {
                // no dtls session
                connP->issecure = 0;
            }
        }
    }

    if (NULL != servinfo) freeaddrinfo(servinfo);
    return connP;
}

void connection_free(dtls_connection_t * connList)
{
    while (connList != NULL) {
        if (connList->issecure != 0) {
        }
        dtls_connection_t * nextP;

        nextP = connList->next;

        if(connList->server_fd) {
            mbedtls_net_free( connList->server_fd );
            lwm2m_free(connList->server_fd);
        }
#ifdef MBEDTLS_ENTROPY_C
        if(connList->entropy) {
            mbedtls_entropy_free( connList->entropy );
            lwm2m_free(connList->entropy);
        }
#endif
         if(connList->conf) {
            mbedtls_ssl_config_free( connList->conf );
            lwm2m_free(connList->conf);
        }
        mbedtls_x509_crt_free( connList->cacert );
        lwm2m_free(connList->cacert);
        if (connList->ssl){
            if(connList->ssl->p_timer)
                lwm2m_free(connList->ssl->p_timer);
            mbedtls_ssl_free( connList->ssl );
            lwm2m_free(connList->ssl);
        }
#ifdef MBEDTLS_ENTROPY_C
        mbedtls_ctr_drbg_free(connList->ctr_drbg );
        lwm2m_free(connList->ctr_drbg);
#endif
        lwm2m_free(connList);
        connList = nextP;
    }
}

int connection_send(dtls_connection_t *connP, uint8_t * buffer, size_t length){

    if (connP->issecure == 0) {
         if ( 0 != send_data(connP, buffer, length)) {
            return -1 ;
        }
    } else {
		if (-1 == mbedtls_ssl_write(connP->ssl,buffer,length)) {
            return -1;
        }
    }
    lwm2m_log(COMP_LOG_DEBUG, "connection_send success\n");
    return 0;
}

int connection_handle_packet(dtls_connection_t *connP, uint8_t * buffer, size_t numBytes){
    if (connP->issecure != 0)//(connP->dtlsSession != NULL)
    {
        lwm2m_log(COMP_LOG_DEBUG, "security mode\n");

	int result = mbedtls_ssl_read(connP->ssl,buffer,numBytes);
	if (result < 0) {
            lwm2m_log(COMP_LOG_ERROR, "error dtls handling message %d\n",result);
	    return result;
        }
	lwm2m_log(COMP_LOG_DEBUG, "after mbedtls_ssl_read,result=%d\n",result);
	if(result > 0)
        {
            lwm2m_handle_packet(connP->lwm2mH, buffer, result, (void*)connP);
        }
        return result;

    } else {
		lwm2m_handle_packet(connP->lwm2mH, buffer, numBytes, (void*)connP);
		lwm2m_log(COMP_LOG_DEBUG, "no securtiy handle packet\n");
        // no security, just return
        return 0;
    }
}

uint8_t lwm2m_buffer_send(void * sessionH,
                          uint8_t * buffer,
                          size_t length,
                          void * userdata)
{
    dtls_connection_t * connP = (dtls_connection_t*) sessionH;

    if (connP == NULL)
    {
        lwm2m_log(COMP_LOG_ERROR, "#> failed sending %lu bytes, missing connection\n", length);
        return COAP_500_INTERNAL_SERVER_ERROR ;
    }

    if (-1 == connection_send(connP, buffer, length))
    {
        lwm2m_log(COMP_LOG_ERROR, "#> failed sending %lu bytes\n", length);
        return COAP_500_INTERNAL_SERVER_ERROR ;
    }
    lwm2m_log(COMP_LOG_DEBUG, "lwm2m_buffer_send success\n");
    return COAP_NO_ERROR;
}

bool lwm2m_session_is_equal(void * session1,
                            void * session2,
                            void * userData)
{
    return (session1 == session2);
}
#endif
