/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include <string.h>

#include "dev_sign_api.h"
#include "mqtt_api.h"
#include "infra_compat.h"
#include "os_task.h"
#include "os_timer.h"
#include "iotx_mqtt_client.h"
#define DBG_EXT_TAG "ali.mqtt.demo"
#define DBG_EXT_LVL DBG_EXT_INFO
#include "dlog.h"
OS_USED static char g_product_key[IOTX_PRODUCT_KEY_LEN + 1]       = PKG_USING_ALI_IOTKIT_PRODUCT_KEY;
OS_USED static char g_product_secret[IOTX_PRODUCT_SECRET_LEN + 1] = PKG_USING_ALI_IOTKIT_PRODUCT_SECRET;
OS_USED static char g_device_name[IOTX_DEVICE_NAME_LEN + 1]       = PKG_USING_ALI_IOTKIT_DEVICE_NAME;
OS_USED static char g_device_secret[IOTX_DEVICE_SECRET_LEN + 1]   = PKG_USING_ALI_IOTKIT_DEVICE_SECRET;
static uint32_t     g_quit                                        = 0;

uint64_t HAL_UptimeMs(void);
int      HAL_Snprintf(char *str, const int len, const char *fmt, ...);
void *   HAL_Malloc(uint32_t size);
void     HAL_Free(void *ptr);
void *   pclient  = NULL;
os_timer_t ali_mqtt_yield_timer;

#ifdef ASYNC_PROTOCOL_STACK

#if OS_TIMER_TASK_STACK_SIZE < 4096

#error "OS_TIMER_TASK_STACK_SIZE need more than 4096 while use feature ASYNC_PROTOCOL_STACK"

#endif 

#endif 


#if 1
static void example_message_arrive(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    iotx_mqtt_topic_info_t *topic_info = (iotx_mqtt_topic_info_pt)msg->msg;

    switch (msg->event_type)
    {
    case IOTX_MQTT_EVENT_PUBLISH_RECEIVED:
        /* print topic name and topic message */
        LOG_I(DBG_EXT_TAG, "Message Arrived:");
        LOG_I(DBG_EXT_TAG, "Topic  : %.*s", topic_info->topic_len, topic_info->ptopic);
        LOG_I(DBG_EXT_TAG, "Payload: %.*s", topic_info->payload_len, topic_info->payload);
        LOG_I(DBG_EXT_TAG, "\n");
        break;
    default:
        break;
    }
}

static int example_subscribe(void *handle)
{
    int         res       = 0;
    const char *fmt       = "/%s/%s/user/get";
    char *      topic     = NULL;
    int         topic_len = 0;

    topic_len = strlen(fmt) + strlen(g_product_key) + strlen(g_device_name) + 1;
    topic     = HAL_Malloc(topic_len);
    if (topic == NULL)
    {
        LOG_E(DBG_EXT_TAG, "memory not enough");
        return -1;
    }
    memset(topic, 0, topic_len);
    HAL_Snprintf(topic, topic_len, fmt, g_product_key, g_device_name);

    res = IOT_MQTT_Subscribe(handle, topic, IOTX_MQTT_QOS0, example_message_arrive, NULL);
    if (res < 0)
    {
        LOG_E(DBG_EXT_TAG, "subscribe failed");
        HAL_Free(topic);
        return -1;
    }

    HAL_Free(topic);
    return 0;
}

/*notice:need define topic on ali iot platform first*/
static int example_publish(void *handle)
{
    int         res       = 0;
    const char *fmt       = "/%s/%s/user/post";
    char *      topic     = NULL;
    int         topic_len = 0;
    char *      payload   = "{\"message\":\"hello!\"}";

    topic_len = strlen(fmt) + strlen(g_product_key) + strlen(g_device_name) + 1;
    topic     = HAL_Malloc(topic_len);
    if (topic == NULL)
    {
        LOG_E(DBG_EXT_TAG, "memory not enough");
        return -1;
    }
    memset(topic, 0, topic_len);
    HAL_Snprintf(topic, topic_len, fmt, g_product_key, g_device_name);

    res = IOT_MQTT_Publish_Simple(0, topic, IOTX_MQTT_QOS0, payload, strlen(payload));
    if (res < 0)
    {
        LOG_E(DBG_EXT_TAG, "publish failed, res = %d", res);
        HAL_Free(topic);
        return -1;
    }

    HAL_Free(topic);
    return 0;
}
#endif
static void example_event_handle(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    LOG_I(DBG_EXT_TAG, "msg->event_type : %d", msg->event_type);
}

extern const char *IOT_Extension_StateDesc(const int);
int                everything_state_handle(const int state_code, const char *state_message)
{
    /*
     * NOTE: Uncomment below to demonstrate how to dump code descriptions
     *
     * After invoking IOT_Extension_StateDesc(), integer code will be translated into strings
     *
     * It looks like:
     *
     * ...
     * everything_state_handle|102 :: recv -0x0329(pub - '/a1MZxOdcBnO/example1/user/get': 0), means 'Report publish
     * relative parameters such as topic string'
     * ...
     *
     */

    /*
        EXAMPLE_TRACE("recv -0x%04X(%s), means '%s'",
                      -state_code,
                      state_message,
                      IOT_Extension_StateDesc(state_code));
    */
    LOG_I(DBG_EXT_TAG, "recv -0x%04X(%s)", -state_code, state_message);
    return 0;
}

#ifdef ASYNC_PROTOCOL_STACK
static void ali_mqtt_async_publish(void *parameter)
{	
    example_publish(pclient);
    IOT_MQTT_Yield(pclient, 200);
}
#endif

#ifdef SUPPORT_TLS
static int identity_response_handle(const char *payload)
{
    LOG_I(DBG_EXT_TAG, "identify: %s", payload);

    return 0;
}
#endif
/*
 *  NOTE: About demo topic of /${productKey}/${deviceName}/user/get
 *
 *  The demo device has been configured in IoT console (https://iot.console.aliyun.com)
 *  so that its /${productKey}/${deviceName}/user/get can both be subscribed and published
 *
 *  We design this to completely demonstrate publish & subscribe process, in this way
 *  MQTT client can receive original packet sent by itself
 *
 *  For new devices created by yourself, pub/sub privilege also requires being granted
 *  to its /${productKey}/${deviceName}/user/get for successfully running whole example
 */

static void mqtt_example_main(void *param)
{
    int               res      = 0;
    int               loop_cnt = 0;
    iotx_mqtt_param_t mqtt_params;
#ifdef DYNAMIC_REGISTER
    int dynamic = 1;
    /* get dynamic option */
    IOT_Ioctl(IOTX_IOCTL_SET_DYNAMIC_REGISTER, (void *)&dynamic);
    IOT_Ioctl(IOTX_IOCTL_SET_PRODUCT_SECRET, g_product_secret);
    IOT_Ioctl(IOTX_IOCTL_SET_PRODUCT_KEY, g_product_key);
    IOT_Ioctl(IOTX_IOCTL_SET_DEVICE_NAME, g_device_name);
#endif

#ifdef SUPPORT_TLS
    IOT_Ioctl(IOTX_IOCTL_SET_MQTT_DOMAIN, "x509.itls.cn-shanghai.aliyuncs.com");
    IOT_Ioctl(IOTX_IOCTL_SET_HTTP_DOMAIN, "iot-auth.cn-shanghai.aliyuncs.com");
    IOT_RegisterCallback(ITE_IDENTITY_RESPONSE, identity_response_handle);
#else
    IOT_Ioctl(IOTX_IOCTL_SET_PRODUCT_KEY, g_product_key);
    IOT_Ioctl(IOTX_IOCTL_SET_DEVICE_NAME, g_device_name);
    IOT_Ioctl(IOTX_IOCTL_SET_DEVICE_SECRET, g_device_secret);
#endif

    IOT_RegisterCallback(ITE_STATE_EVERYTHING, everything_state_handle);

    LOG_I(DBG_EXT_TAG, "mqtt demo start");

    /* Initialize MQTT parameter */
    /*
     * Note:
     *
     * If you did NOT set value for members of mqtt_params, SDK will use their default values
     * If you wish to customize some parameter, just un-comment value assigning expressions below
     *
     **/
    memset(&mqtt_params, 0x0, sizeof(mqtt_params));

    /**
     *
     *  MQTT connect hostname string
     *
     *  MQTT server's hostname can be customized here
     *
     *  default value is ${productKey}.iot-as-mqtt.cn-shanghai.aliyuncs.com
     */
    /* mqtt_params.host = "something.iot-as-mqtt.cn-shanghai.aliyuncs.com";*/

    /**
     *
     *  MQTT connect port number
     *
     *  TCP/TLS port which can be 443 or 1883 or 80 or etc, you can customize it here
     *
     *  default value is 1883 in TCP case, and 443 in TLS case
     */
    /* mqtt_params.port = 1883;*/

    /**
     *
     * MQTT request timeout interval
     *
     * MQTT message request timeout for waiting ACK in MQTT Protocol
     *
     * default value is 2000ms.
     */
    /* mqtt_params.request_timeout_ms = 2000; */

    /**
     *
     * MQTT clean session flag
     *
     * If CleanSession is set to 0, the Server MUST resume communications with the Client based on state from
     * the current Session (as identified by the Client identifier).
     *
     * If CleanSession is set to 1, the Client and Server MUST discard any previous Session and Start a new one.
     *
     * default value is 0.
     */
    /* mqtt_params.clean_session = 0; */

    /**
     *
     * MQTT keepAlive interval
     *
     * KeepAlive is the maximum time interval that is permitted to elapse between the point at which
     * the Client finishes transmitting one Control Packet and the point it starts sending the next.
     *
     * default value is 60000.
     */
    /* mqtt_params.keepalive_interval_ms = 60000; */

    /**
     *
     * MQTT write buffer size
     *
     * Write buffer is allocated to place upstream MQTT messages, MQTT client will be limitted
     * to send packet no longer than this to Cloud
     *
     * default value is 1024.
     *
     */
    /* mqtt_params.write_buf_size = 1024; */

    /**
     *
     * MQTT read buffer size
     *
     * Write buffer is allocated to place downstream MQTT messages, MQTT client will be limitted
     * to recv packet no longer than this from Cloud
     *
     * default value is 1024.
     *
     */
    /* mqtt_params.read_buf_size = 1024; */

    /**
     *
     * MQTT event callback function
     *
     * Event callback function will be called by SDK when it want to notify user what is happening inside itself
     *
     * default value is NULL, which means PUB/SUB event won't be exposed.
     *
     */
    mqtt_params.handle_event.h_fp = example_event_handle;

    pclient = IOT_MQTT_Construct(&mqtt_params);
    if (NULL == pclient)
    {
        LOG_E(DBG_EXT_TAG, "MQTT construct failed");
        return;
    }

#ifdef SUPPORT_TLS
    IOT_Ioctl(IOTX_IOCTL_GET_PRODUCT_KEY, g_product_key);
    IOT_Ioctl(IOTX_IOCTL_GET_DEVICE_NAME, g_device_name);
#endif

#ifdef ASYNC_PROTOCOL_STACK
    iotx_mqtt_nwk_param_t nwk_param;
    IOT_MQTT_Nwk_Event_Handler(pclient,IOTX_MQTT_SOC_CONNECTED,&nwk_param);
    res = example_subscribe(pclient);
    if (res < 0)
    {
        IOT_MQTT_Destroy(&pclient);
        return;
    }
    os_timer_init(&ali_mqtt_yield_timer, "ali_mqtt_yield", ali_mqtt_async_publish, OS_NULL, 1000, OS_TIMER_FLAG_PERIODIC);
    os_timer_start(&ali_mqtt_yield_timer);
    while(!g_quit)
    {
        IOT_MQTT_Nwk_Event_Handler(pclient,IOTX_MQTT_SOC_READ,&nwk_param);
    }
	
#else	
    res = example_subscribe(pclient);
    if (res < 0)
    {
        IOT_MQTT_Destroy(&pclient);
        return;
    }
    while (!g_quit)
    {
        if (0 == loop_cnt % 20)
        {		
            example_publish(pclient);
        }
				
        IOT_MQTT_Yield(pclient, 200);

        loop_cnt += 1;
    }
#endif  //ASYNC_PROTOCOL_STACK
    IOT_MQTT_Destroy(&pclient);

    LOG_I(DBG_EXT_TAG, "mqtt demo stop");

    return;
}
static int mqtt_example_start(int argc, char *argv[])
{
    os_task_t *mqtt_test_hl = os_task_create("ali_mqtt_test", 
                                             mqtt_example_main, 
                                             NULL, 
                                             4096 * 4, 
                                             OS_TASK_PRIORITY_MAX / 2);

    if(NULL == mqtt_test_hl)
    {
        LOG_E(DBG_EXT_TAG, "create task error");  
        return -1;
    }
    else
    {    
        g_quit = 0;
        os_task_startup(mqtt_test_hl);
        return 0;
    }    
}
static int mqtt_example_stop(int argc, char *argv[])
{
    g_quit = 1;
#ifdef ASYNC_PROTOCOL_STACK
    os_timer_stop(&ali_mqtt_yield_timer);
    os_timer_deinit(&ali_mqtt_yield_timer);
#endif
    return 0;
}
#ifdef OS_USING_SHELL
#include <shell.h>
SH_CMD_EXPORT(ali_mqtt_demo, mqtt_example_start, "ali mqtt demo start");
SH_CMD_EXPORT(ali_mqtt_demo_stop, mqtt_example_stop, "ali mqtt demo stop");
#endif /* OS_USING_SHELL */
