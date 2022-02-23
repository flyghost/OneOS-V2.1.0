# Paho MQTT API

------

## 简介

MQTT（Message Queuing Telemetry Transport，消息队列遥测传输协议），是一种基于发布/订阅模式的“轻量级”通讯协议，该协议构建于TCP/IP协议上，由IBM在1999年发布。MQTT最大优点在于，可以以极少的代码和有限的带宽，为连接远程设备提供实时可靠的消息服务。做为一种低开销、低带宽占用的即时通讯协议，使其在物联网、小型设备、移动应用等方面有较广泛的应用。

MQTT组件为用户实现了客户端连接与断开、主题订阅与退订、消息发布、消息接收及回调、心跳维护等功能。

------

## 重要定义及数据结构

### 客户端网络结构体

用于记录服务器地址信息、证书信息、网络操作接口函数等，其定义如下：

```c
typedef struct Network Network;
struct Network
{   
    const char   *pHostAddress;
    uint16_t      port;
    uint16_t      ca_crt_len;
    /* NULL: TCP connection, not NULL: SSL connection */
    const char   *ca_crt;
    /* connection handle: 0 or (uintptr_t)(-1) not connection */
    uintptr_t     handle;
    
    /* function pointer of recv mqtt data */
    int (*mqttread)(Network*, unsigned char *, int, int);
    /* function pointer of send mqtt data */
    int (*mqttwrite)(Network*, unsigned char *, int, int);
    /* function pointer of disconnect mqtt network */
    int (*disconnect)(Network*);
    /* function pointer of establish mqtt network */
    int (*connect)(Network*);
};
```

| 重要成员     | 说明                 |
| ------------ | -------------------- |
| pHostAddress | 服务器地址           |
| port         | 服务器端口号         |
| disconnect   | 断开网络连接操作函数 |
| connect      | 建立网络连接操作函数 |

### 连接选项结构体

用于建立客户端连接时传入相关选项，其定义如下：

```c

typedef struct
{
	/** The eyecatcher for this structure.  must be MQTC. */
	char                   struct_id[4];
	/** The version number of this structure.  Must be 0 */
	int                    struct_version;
	/** Version of MQTT to be used.  3 = 3.1 4 = 3.1.1 */
	unsigned char          MQTTVersion;
	MQTTString             clientID;
	unsigned short         keepAliveInterval;
	unsigned char          cleansession;
	unsigned char          willFlag;
	MQTTPacket_willOptions will;
	MQTTString             username;
	MQTTString             password;
} MQTTPacket_connectData;
```

| 重要成员          | 说明                |
| ----------------- | ------------------- |
| MQTTVersion       | MQTT协议版本号      |
| clientID          | 用户ID              |
| keepAliveInterval | 保活周期            |
| cleansession      | 是否清除session信息 |
| willFlag          | 是否使用临终遗言    |
| will              | 临终遗言信息        |
| username          | 用户名              |
| password          | 用户密钥            |

### 消息结构体

用于描述消息的具体信息，其定义如下：

```c
typedef struct MQTTMessage
{
    enum QoS          qos;
    unsigned char     retained;
    unsigned char     dup;
    unsigned short    id;
    void             *payload;
    size_t            payloadlen;
} MQTTMessage;
```

| 重要成员   | 说明         |
| ---------- | ------------ |
| qos        | 服务等级     |
| retained   | 是否保留     |
| dup        | 重发标识     |
| id         | 消息id       |
| payload    | 消息数据     |
| payloadlen | 消息数据长度 |

### 接收消息处理结构体

用于接收消息的回调函数入参，其定义如下：

```c
typedef struct MessageData
{
    MQTTMessage  *message;
    MQTTString   *topicName;
} MessageData;
```

| 重要成员   | 说明         |
| ---------- | ------------ |
| message    | 消息结构体   |
| topicName  | 主题名称     |

## API列表

| **接口**              | **说明**           |
| :-------------------- | :----------------- |
| MQTTNetworkInit       | MQTT网络层初始化   |
| MQTTNetworkConnect    | MQTT网络连接       |
| MQTTNetworkDisconnect | MQTT网络断开       |
| MQTTClientInit        | MQTT客户端初始化   |
| MQTTClientDeInit      | MQTT客户端反初始化 |
| MQTTConnect           | MQTT建立客户端连接 |
| MQTTIsConnected       | MQTT连接状态查询   |
| MQTTDisconnect        | MQTT断开客户端连接 |
| MQTTSubscribe         | MQTT消息订阅       |
| messageHandler        | 接收消息回调函数   |
| MQTTUnsubscribe       | MQTT消息退订       |
| MQTTPublish           | MQTT消息发布       |
| MQTTYield             | MQTT事务保持       |



## MQTTNetworkInit

该函数用于对MQTT网络结构体进行初始化，其函数原型如下:

```c
int MQTTNetworkInit(Network    *pNetwork,
                    const char *host,
                    uint16_t    port,
                    const char *ca_crt);
```

| **参数** | **说明**                                                     |
| :------- | :----------------------------------------------------------- |
| pNetwork | 需要初始化的网络结构体                                       |
| host     | 连接服务器地址                                               |
| port     | 连接服务器端口                                               |
| ca_crt   | CA证书，只在TLS加密连接下使用，打开宏“MQTT_USING_TLS”进行TLS加密连接则使用证书；关闭宏进行TCP连接，该参数传0。 |
| **返回** | **说明**                                                     |
| int      | =0初始化成功；<0初始化失败                                   |

## MQTTNetworkConnect

该函数用于对MQTT的网络连接，其函数原型如下:

```c
int MQTTNetworkConnect(Network *pNetwork);
```

| **参数** | **说明**               |
| :------- | :--------------------- |
| pNetwork | 需要连接的网络结构体   |
| **返回** | **说明**               |
| int      | =0连接成功；<0连接失败 |

## MQTTNetworkDisConnect

该函数用于对MQTT的网络断开，其函数原型如下:

```c
void MQTTNetworkDisconnect(Network *pNetwork)
```

| **参数** | **说明**               |
| :------- | :--------------------- |
| pNetwork | 需要断开的网络结构体   |
| **返回** | **说明**               |
| void     | =0连接成功；<0连接失败 |

## MQTTClientInit

该函数用于初始化MQTT客户端，其函数原型如下：

```c
void MQTTClientInit(MQTTClient    *c,
                    Network       *network,
                    unsigned int   command_timeout_ms,
                    unsigned char* sendbuf,
                    size_t         sendbuf_size,
                    unsigned char* readbuf,
                    size_t         readbuf_size);
```

| **参数**           | **说明**                                                     |
| :----------------- | :----------------------------------------------------------- |
| c                  | 指向需要初始化的MQTT客户端结构体                             |
| network            | 指向需要初始化的MQTT客户端的网络结构体，在MQTTNetworkInit()中进行初始化 |
| command_timeout_ms | 命令执行超时时间                                             |
| sendbuf            | 客户端发送数据缓冲区地址                                     |
| sendbuf_size       | 发送数据缓冲区容量                                           |
| readbuf            | 客户端接收数据缓冲区地址                                     |
| readbuf_size       | 接收数据缓冲区容量                                           |
| **返回**           | **说明**                                                     |
| 无                 | 无                                                           |

## MQTTClientDeInit

该函数用于反初始化MQTT客户端，其函数原型如下：

```c
void MQTTClientDeInit(MQTTClient* c);
```

| **参数** | **说明**                         |
| :------- | :------------------------------- |
| pNetwork | 指向需要初始化的MQTT客户端结构体 |
| **返回** | **说明**                         |
| void     | 无                               |

## MQTTConnect

该函数用于实现MQTT应用层连接，其函数原型如下：

```c
int MQTTConnect(MQTTClient *c, MQTTPacket_connectData *options);
```

| **参数** | **说明**                   |
| :------- | :------------------------- |
| c        | 客户端结构体指针           |
| options  | 连接选项结构体指针         |
| **返回** | **说明**                   |
| int      | =0初始化成功；<0初始化失败 |

## MQTTIsConnected

该函数用于查询MQTT协议层连接状态，其函数原型如下：

```c
int MQTTIsConnected(MQTTClient* client);
```

| **参数** | **说明**                   |
| :------- | :------------------------- |
| client   | 指向需要查询的客户端结构体 |
| **返回** | **说明**                   |
| int      | =0初始化成功；<0初始化失败 |

## MQTTDisconnect

该函数用于断开MQTT协议链接，该函数原型如下：

```c
int MQTTDisconnect(MQTTClient* c);
```

| **参数** | **说明**                       |
| :------- | :----------------------------- |
| c        | 指向需要断开连接的客户端结构体 |
| **返回** | **说明**                       |
| int      | =0初始化成功；<0初始化失败     |

## MQTTSubscribe

该函数用于实现MQTT消息订阅，该函数原型如下：

```c
int MQTTSubscribe(MQTTClient    *c,
                  const char    *topicFilter,
                  enum QoS       qos,
                  messageHandler messageHandler);
```

| **参数**       | **说明**                       |
| :------------- | :----------------------------- |
| c              | 指向需要订阅消息的客户端结构体 |
| topicFilter    | 指向需要订阅的消息主题         |
| qos            | 订阅消息的服务质量             |
| messageHandler | 指向订阅消息的回调函数         |
| **返回**       | **说明**                       |
| int            | =0初始化成功；<0初始化失败     |

### messageHandler

接收消息回调函数，该函数原型如下：

```c
typedef void (*messageHandler)(MessageData*);
```

| **参数**       | **说明**               |
| :------------- | :--------------------- |
| MessageData    | 接收消息处理结构体     |
| **返回**       | **说明**               |
| 无             |                        |

## MQTTUnsubscribe

该函数用于退订MQTT主题，该函数原型如下：

```c
int MQTTUnsubscribe(MQTTClient* c, const char* topicFilter);
```

| **参数**    | **说明**                       |
| :---------- | :----------------------------- |
| c           | 指向需要退订消息的客户端结构体 |
| topicFilter | 指向需要退订的消息主题         |
| **返回**    | **说明**                       |
| int         | =0初始化成功；<0初始化失败     |

## MQTTPublish

该函数用于发布MQTT消息，该函数原型如下：

```c
int MQTTPublish(MQTTClient *c, const char *topicName, MQTTMessage *message);
```

| **参数**  | **说明**                       |
| :-------- | :----------------------------- |
| c         | 指向需要发布消息的客户端结构体 |
| topicName | 指向需要发布的消息主题         |
| message   | 指向需要发布的消息结构体       |
| **返回**  | **说明**                       |
| int       | =0初始化成功；<0初始化失败     |

## MQTTYield

该函数用于保持MQTT事务，主要进行循环接收数据和心跳保持，该函数原型如下：

```c
int MQTTYield(MQTTClient *c, int timeout_ms);
```

| **参数**   | **说明**                   |
| :--------- | :------------------------- |
| c          | 指向客户端结构体           |
| timeout_ms | 接收阻塞超时时间           |
| **返回**   | **说明**                   |
| int        | =0初始化成功；<0初始化失败 |

