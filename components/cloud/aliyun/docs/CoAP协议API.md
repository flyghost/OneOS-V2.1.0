# CoAP协议API

##### IOT_CoAP_Init

CoAP模块初始化函数, 在使用CoAP的功能之前, 需要使用该函数进行初始化

```c
iotx_coap_context_t *IOT_CoAP_Init(iotx_coap_config_t *p_config);
```

| 参数                  | 说明                  |
| --------------------- | --------------------- |
| p_config              | CoAP模块初始化参数    |
| **返回值**            |                       |
| iotx_coap_context_t * | CoAP当前实例的Context |

###### 参数附加说明

```c
typedef struct {
    char                  *p_url;
    int                   wait_time_ms;
    iotx_device_info_t    *p_devinfo;
    iotx_event_handle_t   event_handle; /* not supported now */
} iotx_coap_config_t;
```

- `p_url`: 需要连接的云端服务器地址
  - 使用DTLS: `coaps://%s.coap.cn-shanghai.link.aliyuncs.com:5684`, `%s` 为ProductKey
  - 使用PSK: `coap-psk://%s.coap.cn-shanghai.link.aliyuncs.com:5682`, `%s` 为ProductKey
- `wait_time_ms`: CoAP收发消息的超时时间
- `p_devinfo`: 设备信息, 包含 Product_Key/ProductSecret/DeviceName 和 DeviceSecret



##### IOT_CoAP_Deinit

CoAP反初始化函数,c 断开与云端的连接并释放所有指定Context中分配的资源

```c
void IOT_CoAP_Deinit(iotx_coap_context_t **pp_context);
```

| 参数       | 说明                       |
| ---------- | -------------------------- |
| pp_context | 需要反初始化的CoAP Context |
| **返回值** |                            |
| 无返回值   |                            |



##### IOT_CoAP_DeviceNameAuth

向云端发送设备认证请求, 认证通过后才能通过CoAP与云端正常通信

```c
int IOT_CoAP_DeviceNameAuth(iotx_coap_context_t *p_context)
```

| 参数       | 说明             |
| ---------- | ---------------- |
| pp_context | CoAP Context     |
| **返回值** |                  |
| 0          | 成功             |
| -1         | 输入参数非法     |
| -2         | 内存不足         |
| -4         | 认证失败         |
| -8         | CoAP消息发送失败 |



##### IOT_CoAP_Yield

当CoAP连接云端后, 用于尝试从网络上接收报文

```c
int IOT_CoAP_Yield(iotx_coap_context_t *p_context)
```

| 参数       | 说明         |
| ---------- | ------------ |
| p_context  | CoAP Context |
| **返回值** |              |
| 0          | 成功         |
| <0         | 失败         |



##### IOT_CoAP_SendMessage

当CoAP连接云端后, 用于向云端发送CoAP消息

```c
int IOT_CoAP_SendMessage(iotx_coap_context_t *p_context, char *p_path, iotx_message_t *p_message);
```

| 参数       | 说明                    |
| ---------- | ----------------------- |
| p_context  | CoAP Context            |
| p_path     | 发送消息的目标资源地址  |
| p_message  | 待发送消息              |
| **返回值** |                         |
| 0          | 成功                    |
| -1         | 输入参数非法            |
| -5         | 设备尚未认证成功        |
| -7         | 待发送消息的payload过长 |

###### 参数附加说明

```c
typedef struct {
    unsigned char            *p_payload;
    unsigned short           payload_len;
    iotx_content_type_t      content_type;
    iotx_msg_type_t          msg_type;
    void                     *user_data;
    iotx_response_callback_t resp_callback;
} iotx_message_t;
```

- `p_payload`: 需要发送的数据内容
- `payload_len`: 需要发送的数据长度
- `content_type`: 数据的格式
- `iotx_msg_type_t`: CoAP消息类型(是否需要Confirmx消息)
- `user_data`: 用户数据, 在收到应答后, 会送回给用户
- `resp_callback`: 用户注册的回调函数, 当收到该消息的应答时被调用



##### IOT_CoAP_GetMessagePayload

当用户通IOT_CoAP_SendMessage送消息并收到应答时, 用该接口可获取CoAP消息中的Payload部分

```c
int IOT_CoAP_GetMessagePayload(void *p_message, unsigned char **pp_payload, int *p_len);
```

| 参数       | 说明                                           |
| ---------- | ---------------------------------------------- |
| p_message  | CoAP应答消息句柄, 在应答消息的回调函数中可获得 |
| pp_payload | 用于存放从`p_message`中获取的消息Payload       |
| p_len      | `pp_payload`的长度                             |
| **返回值** |                                                |
| 0          | 成功                                           |
| -1         | 输入参数非法                                   |
| -2         | 内存不足                                       |



##### IOT_CoAP_GetMessageCode

当用户通 IOT_CoAP_SendMessage送消息并收到应答时, 用该接口可获取CoAP消息中的Code(错误码)部分

```c
int IOT_CoAP_GetMessageCode(void *p_message, iotx_coap_resp_code_t *p_resp_code);
```

| 参数       | 说明                                           |
| ---------- | ---------------------------------------------- |
| p_message  | CoAP应答消息句柄, 在应答消息的回调函数中可获得 |
| pp_payload | 用于存放从`p_message`中获取的消息Code          |
| **返回值** |                                                |
| 0          | 成功                                           |
| -1         | 输入参数非法                                   |

###### 参数附加说明

```c
typedef enum {
    IOTX_COAP_RESP_CODE_CONTENT        = 0x45, /* Mapping to 2.05, Content*/
    IOTX_COAP_RESP_CODE_BAD_REQUEST    = 0x80, /* Mapping to 4.00, Bad Request*/
    IOTX_COAP_RESP_CODE_UNAUTHORIZED   = 0x81, /* Mapping to 4.01, Token is invalid or expire*/
    IOTX_COAP_RESP_CODE_NOT_FOUND      = 0x84, /* Mapping to 4.04, Path or uri is not found*/
    IOTX_COAP_RESP_CODE_URL_TOO_LONG   = 0x8E, /* Mapping to 4.14, The request url is too long*/
    IOTX_COAP_RESP_CODE_INTERNAL_SERVER_ERROR = 0xA0,/* Mapping to 5.00, Internal server error*/

} iotx_coap_resp_code_t;
```

