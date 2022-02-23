#ifndef _CTIOT_DEVICE_INFO_H
#define _CTIOT_DEVICE_INFO_H


#include <oneos_config.h>

#ifdef CTIOT_MQTT_DEVICE_NUMBER
    #define DEVICE_NO CTIOT_MQTT_DEVICE_NUMBER
#else
    #define DEVICE_NO "oneosmqttdevice1"
#endif

#ifdef CTIOT_MQTT_DEVICE_ID
    #define DEVICE_ID CTIOT_MQTT_DEVICE_ID
#else
    #define DEVICE_ID "15011000oneosmqttdevice1"
#endif

#ifdef CTIOT_MQTT_DEVICE_TOKEN
    #define DEVICE_TOKEN CTIOT_MQTT_DEVICE_TOKEN
#else
    #define DEVICE_TOKEN "Mtx0Y5X0RSSyjIbQ-ByYUKTcFGSi7Tfh5YBNLPNy_2U"
#endif

#define DEVICE_PROTOCOL "MQTT"
#define DEVICE_AUTHORIZATION "token"
#define DEVICE_TCPADDRESS "mqtt.ctwing.cn"
#define DEVICE_TCPPORT "1883"
#define DEVICE_TLSADDRESS "mqtt.ctwing.cn"
#define DEVICE_TLSPORT "8883"

#endif //_CTIOT_DEVICE_INFO_H
