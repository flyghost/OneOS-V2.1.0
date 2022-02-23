#if !defined(__CTIOT_MQTT_CONFIG_H)
#define __CTIOT_MQTT_CONFIG_H

#include <oneos_config.h>

#ifndef CONNECT_MODE
#define CONNECT_MODE 1
#endif

#ifdef CTIOT_MQTT_USING_TLS
/* TLS certificate mode */
/* CTIOT security mode config one-way authentication */
#define CTIOT_TLS_SECURITY_ONE_WAY_AUTHENTICATION
/* CTIOT security mode config two-way authentication */
//#define CTIOT_TLS_SECURITY_TWO_WAY_AUTHENTICATION   /* ctiot server not support */
#endif

/*TLS*/
/*TLS双向认证*/
//#define SSL_CERTIFICATE_DUAL
/*TLS单向认证*/
//#define SSL_CERTIFICATE_MONO
/*TLS加密通道*/
//#define SSL_CERTIFICATE_NONE

#endif
