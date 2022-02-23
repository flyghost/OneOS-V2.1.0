# HTTP协议API



##### IOT_HTTP_Init

HTTP模块初始化函数, 在使用HTTP的功能之前, 需要使用该函数进行初始化

```c
void *IOT_HTTP_Init(iotx_http_param_t *pInitParams);
```

| 参数        | 说明               |
| ----------- | ------------------ |
| pInitParams | HTTP模块初始化参数 |
| **返回值**  |                    |
| NULL        | 初始化失败         |
| 非NULL      | HTTP Context       |

###### 参数附加说明

```c
typedef struct {
    iotx_device_info_t *device_info;
    int                 keep_alive;
    int                 timeout_ms;
} iotx_http_param_t;
```

- `device_info`: 设备信息, 包含Product_Key/ProductSecret/DeviceName和DeviceSecret
- `keep_alive`: 选择是否采用http的keep alive模式, 即每次与云端通信完成后是否需要断开http连接
- `timeout_ms:`: 设置等待应答消息的超时时间



##### IOT_HTTP_DeInit

HTTP反初始化函数, 断开与云端的连接并释放所有指定Context中分配的资源

```c
void IOT_HTTP_DeInit(void **handle);
```

| 参数       | 说明         |
| ---------- | ------------ |
| handle     | HTTP Context |
| **返回值** |              |
| 无返回值   |              |



##### IOT_HTTP_DeviceNameAuth

向云端发送设备认证请求, 认证通过后才能通过HTTP与云端正常通信

```c
int IOT_HTTP_DeviceNameAuth(void *handle);
```

| 参数       | 说明         |
| ---------- | ------------ |
| handel     | HTTP Context |
| **返回值** |              |
| 0          | 成功         |
| < 0        | 失败         |



##### IOT_HTTP_SendMessage

当HTTP连接云端后, 用于向云端发送HTTP消息

```c
int IOT_HTTP_SendMessage(void *handle, iotx_http_message_param_t *msg_param);
```

| 参数       | 说明               |
| ---------- | ------------------ |
| handle     | HTTP Context       |
| msg_param  | 待发送到云端的消息 |
| **返回值** |                    |
| 0          | 成功               |
| < 0        | 失败               |

###### 参数附加说明

```c
typedef struct {
    char       *topic_path;
    uint32_t   request_payload_len;
    char       *request_payload;
    uint32_t   response_payload_len;
    char       *response_payload;
    uint32_t   timeout_ms;
} iotx_http_message_param_t;
```

- `topic_path`: 待发送消息的目标资源地址
- `request_payload_len`: 待发送消息的长度
- `request_payload`: 待发送消息的数据
- `response_payload_len`: 应答消息buffer长度
- `response_payload`: 应答消息buffer
- `timeout_ms`: 等待应答消息的超时时间



##### IOT_HTTP_Disconnect

该接口用于断开指定HTTP Context的连接

```c
void IOT_HTTP_Disconnect(void *handle)
```

| 参数       | 说明         |
| ---------- | ------------ |
| handle     | HTTP Context |
| **返回值** |              |
| 无返回值   |              |

