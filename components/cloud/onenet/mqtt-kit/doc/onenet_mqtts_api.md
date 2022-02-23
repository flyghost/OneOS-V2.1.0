# OneNET-MQTT-Kit API

------

## 简介

OneNET-MQTT组件是基于Paho mqtt Embedded C v1.1.0源码包上做的开发，提供自动注册设备、平台接入、主题订阅、消息发布、命令下发、心跳保持、离线重连，并支持TLS加密传输。

------

## 重要定义及数据结构

### g_onenet_info

通过MQTT协议与OneNET平台进行连接的时候，设备端相关的基本信息保存在g_onenet_info结构体中，其定义如下：
```c
typedef struct
{   
    char                           ip[16];
    int                            port;
    char                           pro_id[10];
    char                           access_key[48];
    char                           dev_name[64+1];
    char                           dev_id[16];
    char                           key[48];
    unsigned int                   keepheart_interval;
    unsigned short                 device_register; 
    subscribe_message_handlers_t   subscribe_message_handlers[USER_MESSAGE_HANDLERS_NUM];
} onenet_info_t;
onenet_info_t g_onenet_info;
```
| **重要成员**   			 | **说明**                                 										|
| :------------------------- | :------------------------------------------------------------------------------- |
| ip           	 			 | OneNET平台MQTT连接地址，分为TLS加密连接和TCP非加密连接而不同，本组件已定义       |
| port           			 | 与连接IP对应的端口，本组件已定义               									|
| pro_id         			 | 产品ID，OneNET平台注册MQTT套件后会得到该产品ID                     				|
| access_key     			 | 平台连接key，OneNET平台注册MQTT套件后会得到该access_key 							|
| dev_name       			 | 设备名称，由用户定义，平台有相关定义规则要求                     				|
| dev_id 					 | 设备ID，如果配置开启自动注册，则客户端注册时自动获取，否则在Onenet平台上得到     |
| key      					 | 设备key，如果配置开启自动注册，则客户端注册时自动获取，否则在Onenet平台上得到    |
| keepheart_interval       	 | 心跳间隔时间，由用户定义                     									|
| device_register		 	 | 设备端连接注册状态，1：已注册；0：未注册                               			|
| subscribe_message_handlers | 消息订阅处理函数数组，用以指向不同主题（Topic）的消息处理函数，本组件数组长度为5 |

### g_onenet_mqtts

OneNET-MQTT组件相关的定义实现在g_onenet_mqtts结构体中，包括MQTT网络层相关定义和MQTT客户端相关定义，如下：
```c
typedef struct
{   
    Network      network;
    MQTTClient   client;
} onenet_mqtts_t;
onenet_mqtts_t  g_onenet_mqtts
```
| **重要成员** | **说明**                               	 |
| :----------- | :------------------------------------------ |
| network      | 网络层相关定义结构体，详见Paho-MQTT部分介绍 |
| client       | 客户端相关定义结构体，详见Paho-MQTT部分介绍 |

### onenet_event_t

OneNet-MQTTS所有可能事件由如下枚举体定义：
```c
typedef enum
{   
    ONENET_EVENT_START = 0,
    ONENET_EVENT_DEVICE_REGISTER_OK,
    ONENET_EVENT_DEVICE_REGISTER_FAIL,
    ONENET_EVENT_MQTTS_DEVICE_CONNECTTING,
    ONENET_EVENT_MQTTS_DEVICE_CONNECT_SUCCESS,
    ONENET_EVENT_MQTTS_DEVICE_CONNECT_FAIL,
    ONENET_EVENT_MQTTS_DEVICE_DISCONNECT,
    ONENET_EVENT_KEEP_HEARTBEAT_SUCCESS,
    ONENET_EVENT_SEND_DATA,
    ONENET_EVENT_SUBSCRIBE_SUCCESS,
    ONENET_EVENT_SEND_UNSSUBSCRIBE,
    ONENET_EVENT_PUBLISH_SUCCESS,
    ONENET_EVENT_RECV_CMD,
    ONENET_EVENT_CHECK_MQTTS_DEVICE_STATUS,
    ONENET_EVENT_CHECK_NETWORK,
    ONENET_EVENT_FAULT_PROCESS,

} onenet_event_t;
```
| **重要成员** 								| **说明**                     |
| :---------------------------------------- | :----------------------------|
| ONENET_EVENT_START     					| OneNet-MQTTS流程开始事件 	   |
| ONENET_EVENT_DEVICE_REGISTER_OK    		| 设备注册成功事件         	   |
| ONENET_EVENT_DEVICE_REGISTER_FAIL     	| 设备注册失败事件 		   	   |
| ONENET_EVENT_MQTTS_DEVICE_CONNECTTING     | MQTT设备连接事件     		   |
| ONENET_EVENT_MQTTS_DEVICE_CONNECT_SUCCESS | MQTT设备连接成功事件 		   |
| ONENET_EVENT_MQTTS_DEVICE_CONNECT_FAIL    | MQTT设备连接失败事件 		   |
| ONENET_EVENT_MQTTS_DEVICE_DISCONNECT     	| MQTT设备连接断开事件 		   |
| ONENET_EVENT_KEEP_HEARTBEAT_SUCCESS    	| 心跳保持成功事件       	   |
| ONENET_EVENT_SEND_DATA     				| 数据发送事件 		   		   |
| ONENET_EVENT_SUBSCRIBE_SUCCESS    		| 主题订阅成功事件       	   |
| ONENET_EVENT_SEND_UNSSUBSCRIBE     		| 发送主题退订事件 	   		   |
| ONENET_EVENT_PUBLISH_SUCCESS    			| 主题发布成功事件       	   |
| ONENET_EVENT_RECV_CMD     				| 设备收到下发命令事件 	   	   |
| ONENET_EVENT_CHECK_MQTTS_DEVICE_STATUS    | 检查设备状态事件       	   |
| ONENET_EVENT_CHECK_NETWORK     			| 检查网络层事件 			   |
| ONENET_EVENT_FAULT_PROCESS    			| OneNet-MQTTS流程失败事件     |

### mq_msg
设备通过消息队列发布主题时，主题包含的数据由结构体mq_msg定义，如下：

```c
typedef struct mq_msg_t
{   
    int    topic_type; 
    char   data_buf[128]; 
    int    data_len; 
}mq_msg;
```
| **重要成员** | **说明**            		  |
| :----------- | :--------------------------- |
| topic_type   | 主题类型，包括：    		  |
| 			   | 0：DATA_POINT_TOPIC 		  |
| 			   | 1：DEVICE_IMAGE_GET_TOPIC 	  |
| 			   | 2：DEVICE_IMAGE_UPDATE_TOPIC |
| 			   | 3：CHILD_DEVICE_TOPIC 		  |
| data_buf     | 消息内容数组   			  |
| data_len     | 消息长度			          |

------

## API列表

| **接口**                         | **说明**                     |
| :------------------------------- | :--------------------------- |
| onenet_event_callback            | Onenet发生事件回调函数总接口 |
| onenet_authorization             | Onenet连接Token计算          |
| onenet_get_device_info           | 设备信息获取               |
| onenet_mqtts_init                | Onenet-mqtts初始化           |
| onenet_mqtts_device_is_connected | 设备mqtt连接状态查询       |
| onenet_mqtts_device_register     | 设备mqtt自动注册               |
| onenet_mqtts_device_link         | 设备与Onenet建立连接       |
| onenet_mqtts_device_disconnect   | 设备断开与Onenet连接       |
| onenet_mqtts_device_subscribe    | 设备mqtt消息订阅           |
| onenet_mqtts_client_unsubscribe  | 设备mqtt消息退订           |
| onenet_mqtts_device_publish_cycle| 设备mqtt消息循环发布       |
| onenet_mqtts_device_publish      | 设备mqtt消息发布           |
| onenet_mqtts_publish             | 发布数据发送到mqtt消息队列   |
| onenet_mqtts_device_start        | 设备mqtt流程开始           |
| onenet_mqtts_device_end          | 设备mqtt流程结束           |



## onenet_event_callback

Onenet接入时发生事件的回调函数总接口，对于不同的事件，用户可以在该函数内注册自己所需的回调处理函数。其函数原型如下:

```c
void onenet_event_callback(onenet_event_t onenet_event);
```

| **参数**      | **说明**                             	    |
| :------------ | :---------------------------------------- |
| onenet_event  | 发生事件的名称，由以上onenet_event_t中定义|
| **返回**      | **说明**                          	    |
| void		    | 无		                                |

## onenet_authorization

计算设备注册或连接时需要用到的Token令牌，当参数et设置时间小于当前时间时，onenet平台会认为Token令牌过期从而拒绝该访问。其函数原型如下:

```c
int onenet_authorization(char             *ver,
						 char             *res,
						 unsigned int     et,
						 char             *access_key,
						 char             *dev_name,
						 char             *authorization_buf,
						 unsigned short   authorization_buf_len,
						 _Bool            flag);
```

| **参数**			   | **说明**     									|
| :------------------- | :--------------------------------------------- |
| ver 				   | 参数组版本号，日期格式，目前仅支持"2018-10-31" |
| res 				   | 产品ID，OneNET平台注册MQTT套件后会得到该产品ID |
| et   				   | Token过期时间expirationTime，unix时间 			|
| access_key   		   | API访问时是产品access_key；设备连接时是设备key |
| dev_name 			   | API访问时传NULL；设备连接时是本设备的设备名称  |
| authorization_buf    | 计算所得Token字符串的缓存区 					|
| authorization_buf_len| Token字符串的缓存区长度 						|
| flag   			   | 1：用于计算API访问的Token（注册时用） 			|
| 		   			   | 0：用于计算设备连接的Token					 	|
| **返回** 			   | **说明**    								    |
| 0					   | 计算成功         								|
| 1					   | 计算失败   							        |

## onenet_get_device_info

设备信息获取，获得连接所需的PRODUCT_ID、ACCESS_KEY、DEVICE_NAME、DEVICE_ID、USER_KEY、心跳间隔等信息，这些信息由用户事先在"onenet_device_sample.h"头文件中填写。函数原型如下：

```c
int onenet_get_device_info(void);
```

| **参数** | **说明** |
| :------- | :------- |
| void     | 无		  |
| **返回** | **说明** |
| OS_TRUE  | 获取成功 |
| OS_FALSE | 获取失败 |

## onenet_mqtts_init

Onenet-mqtts初始化，包括网络、客户端结构、消息订阅主题（topic）的初始化。函数原型如下：

```c
void onenet_mqtts_init(void);
```

| **参数** | **说明** |
| :------- | :------- |
| void     | 无  	  |
| **返回** | **说明** |
| void	   | 无	      |

## onenet_mqtts_device_is_connected

设备mqtt连接状态查询，指的mqtt协议层连接状态。其函数原型如下：

```c
int onenet_mqtts_device_is_connected(void);
```

| **参数** | **说明**         |
| :------- | :--------------- |
| void     | 无  		      |
| **返回** | **说明**         |
| 0	  	   | 已断开           |
| 1	  	   | 已连接           |

## onenet_mqtts_device_register

用于onenet-mqtt自动注册功能，设备连接注册服务器，发送注册信息，接收注册回复信息，最后断开注册服务器网络。设备名需要保证唯一，建议采用SN、IMEI等，支持数字、字母、字符'_'和'-'，长度不超过64。其函数原型如下：

```c
int onenet_mqtts_device_register(const char   *access_key,
								 const char   *pro_id,
								 const char   *serial,
								 char         *dev_id,
								 char         *key);
```

| **参数**	 | **说明**     					   							    	   |
| :----------| :---------------------------------------------------------------------- |
| access_key | OneNET平台注册MQTT套件后得到该access_key，可由onenet_get_device_info获得|
| pro_id 	 | OneNET平台注册MQTT套件后得到该ID，可由onenet_get_device_info获得  	   |
| serial   	 | 本设备的设备名称，可由onenet_get_device_info获得						   |
| dev_id 	 | 存储平台返回的设备ID字符串的缓存区 		    						   |
| key    	 | 存储平台返回的设备key字符串的缓存区									   |
| **返回** 	 | **说明**    								    						   |
| OS_FALSE	 | 注册成功         													   |
| OS_TRUE	 | 注册失败   							        						   |

## onenet_mqtts_device_link

设备与OneNET平台建立MQTT连接，包括网络层和协议层连接。网络连接失败会直接返回，设备接入OneNET失败会先断开网络再返回。其函数原型如下：

```c
int onenet_mqtts_device_link(void);
```

| **参数** | **说明** |
| :------- | :------- |
| void     | 无		  |
| **返回** | **说明** |
| OS_FALSE | 接入失败 |
| OS_TRUE  | 接入成功 |

## onenet_mqtts_device_disconnect

设备断开与OneNET的MQTT连接，该函数只断开协议层连接。其函数原型如下：

```c
void onenet_mqtts_device_disconnect(void);
```

| **参数** | **说明** |
| :------- | :------- |
| void     | 无  	  |
| **返回** | **说明** |
| void	   | 无	      |

## onenet_mqtts_device_subscribe

设备mqtt消息订阅，订阅在onenet_mqtts_init()中已初始化的主题（Topic），目前最大值设置为5个，注册Topic对应的消息回调函数。每个主题订阅最多尝试3次，非法订阅平台会断开连接。该函数原型如下：

```c
int onenet_mqtts_device_subscribe(void);
```

| **参数** | **说明** |
| :------- | :------- |
| void     | 无		  |
| **返回** | **说明** |
| OS_FALSE | 订阅失败 |
| OS_TRUE  | 订阅成功 |

## onenet_mqtts_client_unsubscribe

设备mqtt消息退订，退订topicFilter指定主题（Topic）的消息。其函数原型如下：

```c
int onenet_mqtts_client_unsubscribe(const char *topicFilter);
```

| **参数**    | **说明** 		   |
| :---------- | :----------------- |
| topicFilter | 需要退订的主题名称 |
| **返回** 	  | **说明** 		   |
| OS_FALSE 	  | 退订失败    	   |
| OS_TRUE     | 退订成功 		   |

## onenet_mqtts_device_publish_cycle

设备mqtt消息循环发布，在每次发布间隔时间超过预置定时时间（可在"onenet_device_sample.h"头文件中设置，预置为10s）后，从本函数中构建的示例缓存区中获取消息数据并进行发布。其函数原型如下：

```c
int onenet_mqtts_device_publish_cycle(os_tick_t *last_publish_tick)
```

| **参数**    		| **说明** 		   				  |
| :---------------- | :------------------------------ |
| last_publish_tick | 上次发布时的时间，以cycle为单位 |
| **返回** 	 	    | **说明** 			   		      |
| OS_FALSE 	  		| 发布失败    	   				  |
| OS_TRUE     		| 发布成功 		   				  |

## onenet_mqtts_device_publish

设备mqtt消息发布，从预置的MQTT消息队列获取消息数据并发布，预置队列消息需要包含发布的主题，数据，数据长度。消息的服务质量默认为QOS1，平台支持QOS0、QOS1、不支持QOS2。其函数原型如下：

```c
int onenet_mqtts_device_publish(void);
```

| **参数** | **说明** |
| :------- | :------- |
| void     | 无		  |
| **返回** | **说明** |
| OS_FALSE | 发布失败 |
| OS_TRUE  | 发布成功 |

## onenet_mqtts_publish

MSH接口输入命令，将想要发布的固定格式的mqtt消息数据发送到预置的mqtt消息队列，以进行发布。
MSH命令的输入参数只能是3个：
第一个参数：onenet_mqtts_publish；
第二个参数：0-数据点或 1-获取镜像或 2-更新镜像；
第三个参数：要发布的数据。
例如：onenet_mqtts_publish 0 {"id":101,"dp":{"humi":[{"v":32,}],"temp":[{"v":25,}]}}
其函数原型如下：

```c
void onenet_mqtts_publish(int argc,  char *argv[]);
```

| **参数** | **说明**   |
| :------- | :--------- |
| argc     | 传参个数 	|
| argv[]   | 参数值数据 |
| **返回** | **说明**   |
| void 	   | 无   		|

## onenet_mqtts_device_start

MSH接口输入命令，该函数预置一个完整的Onenet-MQTT接入并保持范例的框架，包括mqtt独立线程初始化，网络、客户端和消息队列初始化，客户端注册，网络连接，客户端连接，预置主题消息订阅、发布及心跳保持等功能，详见使用示例介绍。
MSH命令为：onenet_mqtts_device_start
其函数原型如下：

```c
void onenet_mqtts_device_start(void);
```

| **参数** | **说明** |
| :------- | :------- |
| void     | 无  	  |
| **返回** | **说明** |
| void	   | 无	      |

## onenet_mqtts_device_end

MSH接口输入命令，对onenet_mqtts_device_start函数启动的mqtt流程的结束退出。
MSH命令为：onenet_mqtts_device_end
其函数原型如下：

```c
void onenet_mqtts_device_end(void);
```

| **参数** | **说明** |
| :------- | :------- |
| void     | 无  	  |
| **返回** | **说明** |
| void	   | 无	      |

