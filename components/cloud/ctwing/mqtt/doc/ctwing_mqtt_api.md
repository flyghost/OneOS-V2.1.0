# CTWing-MQTT API

------

## 简介

CTWing-MQTT组件是OneOS对天翼物联网平台MQTT接入SDK的适配与开发，支持明文传输和TLS加密传输、支持设备消息透传模式和使能服务模式。

------

## 重要定义及数据结构

### mqtt_param_s ctiot_params;

与平台建立网络连接时的网络相关信息，其定义如下：
```c
typedef struct
{
    char *server_ip;
    char *server_port;
    mqtt_security_info_s info;
}mqtt_param_s;
```
| **重要成员**   			 | **说明**                                 										|
| :------------------------- | :------------------------------------------------------------------------------- |
| server_ip        | 服务器域名或IP |
| server_port    | 服务器端口号 |
| info | 安全连接相关信息 |

### mqtt_device_info_s device_info;

与平台建立MQTT连接时的设备相关信息，其定义如下：

```c
typedef struct
{
    mqtt_connection_type_e connection_type;
    mqtt_codec_mode_e codec_mode;
    char *password;
    union
    {
        mqtt_static_connection_info_s s_info;
    }u;
}mqtt_device_info_s;
```
| **重要成员**    | **说明**                       |
| :-------------- | :----------------------------- |
| connection_type | 连接类型（只支持静态连接）     |
| codec_mode      | 数据编码格式（只支持json格式） |
| password        | 密钥（平台的特征串）           |
| u               | 设备ID信息                     |

### mqtt_client_s *g_phandle;

与平台建立MQTT连接时的操作句柄，其定义如下：

```c
struct mqtt_client_tag_s
{
	mqtt_device_info_s device_info;
	MQTTClient client;
	mqtt_param_s params;
	void *ctiotCbFunc;
	char *sub_topic;
	uint8_t init_flag;
	uint8_t reserve[3];
};
```

| **重要成员** | **说明**           |
| :----------- | :----------------- |
| device_info  | 设备相关信息       |
| client       | MQTT客户端相关信息 |
| params       | 网络相关信息       |
| ctiotCbFunc  | 命令回调函数指针   |
| sub_topic    | 订阅主题           |
| init_flag    | 初始化状态         |
| reserve      | 保留               |

## API列表

| **接口**                                             | **说明**                     |
| :--------------------------------------------------- | :--------------------------- |
| ctiot_mqtt_init                                      | 参数初始化                   |
| ctiot_mqtt_login                                     | 设备登录平台                 |
| ctiot_mqtt_logout                                    | 设备登出平台                 |
| ctiot_mqtt_subscribe                                 | 设备订阅                     |
| ctiot_mqtt_msg_publish                               | 设备发布                     |
| ctiot_handleMessagr                                  | 消息接收、回调及心跳维护     |
| ctiot_mqtt_isconnected                               | 查看是否连接                 |
| ctiot_mqtt_encode_data_report_service_datareport     | 封装json格式数据上报报文     |
| ctiot_mqtt_encode_event_report_service_eventreport   | 封装json格式事件上报报文     |
| ctiot_mqtt_encode_cmd_response_service_cmddnresponse | 封装json格式命令响应报文     |
| ctiot_init_heap                                      | 在系统中申请较大的堆内存     |
| ctiot_free_all_heap                                  | 在系统中释放较大的堆内存     |
| OS_GET_MEM                                           | 在较大的堆内存中申请内存使用 |
| OS_PUT_MEM                                           | 在较大的堆内存中释放内存使用 |

## ctiot_mqtt_init

参数初始化

```c
int ctiot_mqtt_init(const mqtt_param_s *params, void *callback_struct, mqtt_client_s **phandle);
```

| **参数**        | **说明**                    |
| :-------------- | :-------------------------- |
| params          | 初始化结构体                |
| callback_struct | 回调函数结构体              |
| phandle         | 返回初始化的mqtt_client句柄 |
| **返回**        | **说明**                    |
| 0               | 成功                        |
| < 0             | 失败                        |

## ctiot_mqtt_login

设备登录平台

```c
int ctiot_mqtt_login(const mqtt_device_info_s* device_info, mqtt_client_s* phandle);
```

| **参数**    | **说明**        |
| :---------- | :-------------- |
| device_info | 设备信息结构体  |
| phandle     | mqtt_client句柄 |
| **返回**    | **说明**        |
| 0           | 成功            |
| < 0         | 失败            |

## ctiot_mqtt_logout

设备登出平台

```c
CTIOT_STATUS ctiot_mqtt_logout(mqtt_client_s* phandle);
```

| **参数** | **说明**        |
| :------- | :-------------- |
| phandle  | mqtt_client句柄 |
| **返回** | **说明**        |
| 0        | 成功            |
| < 0      | 失败            |

## ctiot_mqtt_subscribe

设备订阅

```c
int ctiot_mqtt_subscribe(void* mhandle);
```

| **参数** | **说明**        |
| :------- | :-------------- |
| mhandle  | mqtt_client句柄 |
| **返回** | **说明**        |
| 0        | 成功            |
| < 0      | 失败            |

## ctiot_mqtt_msg_publish

设备发布

```c
CTIOT_MSG_STATUS ctiot_mqtt_msg_publish(char *topic, mqtt_qos_e qos, char* payload);
```

| **参数** | **说明**     |
| :------- | :----------- |
| topic    | 发布主题     |
| qos      | 消息质量等级 |
| payload  | 消息负载     |
| **返回** | **说明**     |
| 0        | 成功         |
| > 0      | 失败         |

## ctiot_handleMessagr

消息接收、回调及心跳维护

```c
CTIOT_STATUS ctiot_handleMessagr(mqtt_client_s* phandle);
```

| **参数** | **说明**        |
| :------- | :-------------- |
| phandle  | mqtt_client句柄 |
| **返回** | **说明**        |
| 0        | 成功            |
| < 0      | 失败            |

## ctiot_mqtt_isconnected

查看是否连接

```c
int ctiot_mqtt_isconnected(mqtt_client_s* phandle);
```

| **参数** | **说明**        |
| :------- | :-------------- |
| phandle  | mqtt_client句柄 |
| **返回** | **说明**        |
| true     | 成功            |
| false    | 失败            |

## ctiot_mqtt_encode_data_report_service_datareport

封装json格式数据上报报文

```c
CTIOT_MSG_STATUS ctiot_mqtt_encode_data_report_service_datareport(DATA_REPORT_SERVICE_DATAREPORT* para,char** payload);
```

| **参数** | **说明**                 |
| :------- | :----------------------- |
| para     | 上报数据                 |
| payload  | 将上报数据转化为json格式 |
| **返回** | **说明**                 |
| 0        | 成功                     |
| > 0      | 失败                     |

## ctiot_mqtt_encode_event_report_service_eventreport

封装json格式事件上报报文

```c
CTIOT_MSG_STATUS ctiot_mqtt_encode_event_report_service_eventreport(EVENT_REPORT_SERVICE_EVENTREPORT* para,char** payload)；
```

| **参数** | **说明**                 |
| :------- | :----------------------- |
| para     | 上报事件                 |
| payload  | 将上报事件转化为json格式 |
| **返回** | **说明**                 |
| 0        | 成功                     |
| > 0      | 失败                     |

## ctiot_mqtt_encode_cmd_response_service_cmddnresponse

封装json格式命令响应报文

```c
CTIOT_MSG_STATUS ctiot_mqtt_encode_cmd_response_service_cmddnresponse(CMD_RESPONSE_SERVICE_CMDDNRESPONSE* para,char** payload);
```

| **参数** | **说明**                 |
| :------- | :----------------------- |
| para     | 上报事件                 |
| payload  | 将响应数据转化为json格式 |
| **返回** | **说明**                 |
| 0        | 成功                     |
| > 0      | 失败                     |

## ctiot_init_heap

在系统中申请较大的堆内存

```c
SDK_S16 ctiot_init_heap(SDK_U32 blockSize, SDK_U32 blockNumber)
```

| **参数**    | **说明** |
| :---------- | :------- |
| blockSize   | 块大小   |
| blockNumber | 块数目   |
| **返回**    | **说明** |
| 0           | 成功     |
| -1          | 失败     |

## ctiot_free_all_heap

在系统中释放较大的堆内存

```c
void ctiot_free_all_heap(void)
```

| **参数** | **说明** |
| :------- | :------- |
| void     | 无       |
| **返回** | **说明** |
| void     | 无       |

## OS_GET_MEM

在较大的堆内存中申请内存使用

```c
OS_GET_MEM(PTR,TYPE,SIZE)
```

| **参数** | **说明**   |
| :------- | :--------- |
| PTR      | 传参个数   |
| TYPE     | 参数值数据 |
| SIZE     | 申请大小   |
| **返回** | **说明**   |
| 无       | 无         |

## OS_PUT_MEM

在较大的堆内存中释放内存使用

```c
OS_PUT_MEM(PTR)
```

| **参数** | **说明** |
| :------- | :------- |
| PTR      | 地址     |
| **返回** | **说明** |
| void     | 无       |