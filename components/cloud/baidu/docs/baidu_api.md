# Baidu Cloud API

## 简介

百度云组件是OneOS操作系统提供的百度云接入组件，是对百度云发布的适用于嵌入式设备的SDK的移植与开发，通过主流的物联网协议（如 MQTT）通讯，可以在智能设备与云端之间建立安全的双向连接，快速实现物联网项目。

## API接口

| 函数名称                      | 说明                                   |
| ----------------------------- | -------------------------------------- |
| initialize_mqtt_client_handle | MQTT客户端初始化                       |
| iotcore_mqtt_doconnect        | MQTT客户端连接到服务器                 |
| publish_mqtt_message          | MQTT客户端发布消息                     |
| subscribe_mqtt_topic          | MQTT客户端订阅消息                     |
| unsubscribe_mqtt_topics       | MQTT客户端去订阅消息                   |
| iotcore_mqtt_dowork           | MQTT客户端主循环，需要在线程中循环调用 |
| iotcore_mqtt_disconnect       | MQTT客户端断开连接                     |
| iotcore_mqtt_destroy          | MQTT客户端销毁资源                     |
| set_certificates              | MQTT客户端设置服务器证书               |
| set_client_cert               | MQTT客户端设置设备证书及私钥           |
| iotcore_get_mqtt_status       | MQTT客户端连接状态查询                 |



### initialize_mqtt_client_handle

mqtt客户端初始化函数，初始化成功后，返回MQTT client操作句柄。

```c
IOTCORE_MQTT_CLIENT_HANDLE initialize_mqtt_client_handle(const IOTCORE_INFO 	*info, 
                                                         const char 			*will_topic, 
                                                         const char 			*will_payload,
                                                         MQTT_CONNECTION_TYPE 	conn_type,
                                                         RECV_MSG_CALLBACK 		callback,
                                                         IOTCORE_RETRY_POLICY 	retry_policy,
                                                         size_t 				retry_timeout_limit_in_sec);
```

| 参数                       | 说明                                                         |
| -------------------------- | ------------------------------------------------------------ |
| info                       | 设备注册相关参数，包含：产品ID、设备ID、设备密钥(psk模式)、云服务器地址 |
| will_topic                 | 遗嘱消息主题，需要时添加                                     |
| will_payload               | 遗嘱消息内容，需要时添加                                     |
| conn_type                  | 连接类型，包含MQTT、MQTTS-PSK、MQTTS-CA三种类型              |
| callback                   | 消息接收回调函数，当收到平台下发消息时，调用该函数处理，需要用户实现 |
| retry_policy               | 重连接策略，包括：无重连、间隔时间、线性退避等策略           |
| retry_timeout_limit_in_sec | 重连接最大超时时间，单位秒                                   |
| **返回**                   | **说明**                                                     |
| NULL                       | 执行失败                                                     |
| 非NULL                     | MQTT client操作句柄                                          |



### iotcore_mqtt_doconnect

mqtt客户端连接云服务器，包括mqtts下的TLS握手及平台注册、鉴权等操作。

```c
int iotcore_mqtt_doconnect(IOTCORE_MQTT_CLIENT_HANDLE iotcore_client, size_t timeout);
```

| 参数           | 说明                 |
| -------------- | -------------------- |
| iotcore_client | mqtt客户端操作句柄   |
| timeout        | 连接超时时间，单位秒 |
| **返回**       | **说明**             |
| 0              | 执行成功             |
| 非0            | 执行失败，返回错误码 |



### publish_mqtt_message

mqtt客户端消息发布，用以客户端向平台发布消息。

```c
int publish_mqtt_message(IOTCORE_MQTT_CLIENT_HANDLE iotcore_client,
                         const char 				*pub_topic_name,
                         IOTCORE_MQTT_QOS 			qos_value,
                         const uint8_t 				*pub_msg,
                         size_t 					pub_msg_length,
                         PUB_CALLBACK 				handle,
                         void 						*context);
```

| 参数           | 说明                                                         |
| -------------- | ------------------------------------------------------------ |
| iotcore_client | mqtt客户端操作句柄                                           |
| pub_topic_name | 发布消息的主题                                               |
| qos_value      | QoS策略，支持QoS0和QoS1                                      |
| pub_msg        | 需要发布的消息内容地址                                       |
| pub_msg_length | 消息长度                                                     |
| handle         | 消息发布结果回调函数，消息发布完成后将执行该函数，用户可以根据需要来实现该函数 |
| context        | 调用handle回调时，传入的上下文                               |
| **返回**       | **说明**                                                     |
| 0              | 执行成功                                                     |
| 非0            | 执行失败，返回错误码                                         |



### subscribe_mqtt_topic

mqtt客户端消息订阅，用以客户端向平台订阅消息。

```c
int subscribe_mqtt_topic(IOTCORE_MQTT_CLIENT_HANDLE iotcore_client, 
                         const char					*sub_topic,
                         IOTCORE_MQTT_QOS 			ret_qos,
                         SUB_CALLBACK 				sub_callback,
                         void* 						context);
```

| 参数           | 说明                                                         |
| -------------- | ------------------------------------------------------------ |
| iotcore_client | mqtt客户端操作句柄                                           |
| sub_topic      | 需要订阅的主题                                               |
| ret_qos        | 服务质量要求，支持QoS0和QoS1                                 |
| sub_callback   | 消息订阅完成后回调函数，消息订阅完成后将执行该函数，用户可以根据需要来实现该函数 |
| context        | 调用sub_callback回调时，传入的上下文                         |
| **返回**       | **说明**                                                     |
| 0              | 执行成功                                                     |
| 非0            | 执行失败，返回错误码                                         |



### unsubscribe_mqtt_topics

mqtt客户端消息去订阅，用以客户端向平台消息去订阅。

```c
int unsubscribe_mqtt_topics(IOTCORE_MQTT_CLIENT_HANDLE iotcore_client, const char* unsubscribe);
```

| 参数           | 说明                 |
| -------------- | -------------------- |
| iotcore_client | mqtt客户端操作句柄   |
| unsubscribe    | 需要去订阅的主题     |
| **返回**       | **说明**             |
| 0              | 执行成功             |
| 非0            | 执行失败，返回错误码 |



### iotcore_mqtt_dowork

mqtt客户端主循环，客户端连接到服务器后，需要在线程或任务中循环调用。

```c
void iotcore_mqtt_dowork(IOTCORE_MQTT_CLIENT_HANDLE iotcore_client);
```

| 参数           | 说明               |
| -------------- | ------------------ |
| iotcore_client | mqtt客户端操作句柄 |
| **返回**       | **说明**           |
| void           |                    |



### iotcore_mqtt_disconnect

mqtt客户端断开连接，用以客户端主动断开连接

```c
int iotcore_mqtt_disconnect(IOTCORE_MQTT_CLIENT_HANDLE iotcore_client);
```

| 参数           | 说明                 |
| -------------- | -------------------- |
| iotcore_client | mqtt客户端操作句柄   |
| **返回**       | **说明**             |
| 0              | 执行成功             |
| 非0            | 执行失败，返回错误码 |



### iotcore_mqtt_destroy

释放mqtt客户端操作句柄所占用的资源，释放后该指针不能使用。

```c
void iotcore_mqtt_destroy(IOTCORE_MQTT_CLIENT_HANDLE iotcore_client);
```

| 参数           | 说明               |
| -------------- | ------------------ |
| iotcore_client | mqtt客户端操作句柄 |
| **返回**       | **说明**           |
| void           |                    |



### set_certificates

设置百度云服务器证书，用于TLS握手阶段服务器证书比对。

```c
void set_certificates(IOTCORE_MQTT_CLIENT_HANDLE iotcore_client, const char* certificates);
```

| 参数           | 说明               |
| -------------- | ------------------ |
| iotcore_client | mqtt客户端操作句柄 |
| certificates   | 服务器证书储存地址 |
| **返回**       | **说明**           |
| void           |                    |



### set_client_cert

设置设备证书及私钥，用于TLS握手阶段上传服务器。

```c
void set_client_cert(IOTCORE_MQTT_CLIENT_HANDLE iotcore_client, 
                     const char					*client_cert, 
                     const char					*client_key);
```

| 参数           | 说明               |
| -------------- | ------------------ |
| iotcore_client | mqtt客户端操作句柄 |
| client_cert    | 设备证书储存地址   |
| client_key     | 设备私钥存储地址   |
| **返回**       | **说明**           |
| void           |                    |



### iotcore_get_mqtt_status

mqtt客户端连接状态查询，查询当前连接状态。

```c
IOTCORE_MQTT_STATUS iotcore_get_mqtt_status(IOTCORE_MQTT_CLIENT_HANDLE iotcore_client);
```

| 参数                | 说明               |
| ------------------- | ------------------ |
| iotcore_client      | mqtt客户端操作句柄 |
| **返回**            | **说明**           |
| IOTCORE_MQTT_STATUS | 返回当前连接状态   |
