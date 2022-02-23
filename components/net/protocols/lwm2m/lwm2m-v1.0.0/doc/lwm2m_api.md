# lwm2m组件

## 简介

LwM2M全称lightweight Machine to Machine,是OMA（Open Moblile Alliance）定义的物联网协议，主要可以使用在资源受限（包括存储、功耗等）的嵌入式设备。这个协议基于CoAP（Constrained Application Protocol）协议，CoAP协议基于UDP协议，默认UDP端口号非加密为5683，加密端口5684。Eclipse Wakaama：基于C，提供了LwM2M服务器与LwM2M客户端的实现。

## API列表

| **接口**                  | 说明                           |
| :------------------------ | ------------------------------ |
| lwm2m_init                | 初始化一个lwm2m上下文          |
| lwm2m_deinit              | 关闭一个lwm2m上下文            |
| lwm2m_step                | 执行任何请求处理操作           |
| lwm2m_handle_packet       | 发送收到的数据到liblwm2m库处理 |
| lwm2m_configure           | 配置匹配的服务器对象实例       |
| lwm2m_add_object          | 添加对象实例                   |
| lwm2m_remove_object       | 去除对象实例                   |
| lwm2m_update_registration | 发送更新注册信息到指定的服务器 |

## API说明

### lwm2m_init

该函数用于初始化lwm2m上下文运行环境

```c
void* lwm2m_init(void * userData)
```

| 参数     | 说明                               |
| -------- | ---------------------------------- |
| userData | 用于存放用户相关数据格式到上下文中 |

### lwm2m_deinit

该函数用于删除链表和内存释放相关操作

```c
void lwm2m_deinit(void * handler)
```

| 参数    | 说明               |
| ------- | ------------------ |
| handler | 上下文运行环境变量 |

### lwm2m_step

该函数用于执行任何请求处理操作

```c
int lwm2m_step(void *handler,time_t * timeoutP)
```

| 参数     | 说明               |
| -------- | ------------------ |
| handler  | 上下文运行环境变量 |
| timeoutP | 请求超时变量       |

### lwm2m_handle_packet

该函数用于发送收到的数据到liblwm2m库进一步处理

```c
void lwm2m_handle_packet(void* handler, uint8_t * buffer, int length, void *fromSessionH)
```

| 参数         | 说明               |
| ------------ | ------------------ |
| handler      | 上下文运行环境变量 |
| buffer       | 指向CoAP包的头指针 |
| length       | 收到的字节数       |
| fromSessionH | 存放socket连接信息 |

### lwm2m_configure

该函数用于配置匹配的服务器对象实例

```c
int lwm2m_configure(void * handler,const char * endpointName,const char * msisdn,const char * altPath,uint16_t numObject,lwm2m_object_t * objectList[])
```

| 参数         | 说明               |
| ------------ | ------------------ |
| handler      | 上下文运行环境变量 |
| endpointName | 终端名字           |
| msisdn       | 移动用户号码       |
| altPath      | 路径更改           |
| numObject    | 对象数量           |
| objectList[] | 对象链表           |

### lwm2m_add_object

该函数用于添加对象实例到运行环境中

```c
int lwm2m_add_object(void* handler, lwm2m_object_t * objectP)
```

| 参数    | 说明               |
| ------- | ------------------ |
| handler | 上下文运行环境变量 |
| objectP | 对象指针           |

### lwm2m_remove_object

该函数用于从运行环境中删除对象实例

```c
int lwm2m_remove_object(void* handler, uint16_t id)
```

| 参数    | 说明               |
| ------- | ------------------ |
| handler | 上下文运行环境变量 |
| id      | 对象ID             |

### lwm2m_update_registration

该函数用于更新已给的服务注册信息

```c
int lwm2m_update_registration(void* handler, uint16_t shortServerID, bool withObjects)
```

| 参数          | 说明               |
| ------------- | ------------------ |
| handler       | 上下文运行环境变量 |
| shortServerID | 服务器短ID         |
| withObjects   | 是否带有对象       |

