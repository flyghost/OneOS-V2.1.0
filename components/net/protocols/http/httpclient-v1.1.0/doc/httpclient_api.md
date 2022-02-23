# HTTP组件

## 简介

HTTP协议永远都是客户端发起请求，服务器回送响应。这样就限制了使用HTTP协议，无法实现在客户端没有发起请求的时候，服务器将消息推送给客户端。

本例程介绍HTTP Client发送HTTP协议GET/POST/PUT/DELETE/HEAD请求到服务器，并接受响应数据的整个过程实现方法和步骤。分别对应着下载、上传、指定位置上传、删除、下载不含响应信息。

## API列表

| **接口**           | **说明**                                             |
| :----------------- | :--------------------------------------------------- |
| http_client_get    | HTTP组件用于客户端访问服务器网站或者下载文件         |
| http_client_post   | HTTP组件用于客户端访问服务器网站上传数据或者上传文件 |
| http_client_put    | HTTP组件用户客户端访问服务器指定位置上传文件         |
| http_client_head   | HTTP组件用于客户端访问服务器下载不含响应信息消息     |
| http_client_delete | HTTP组件用于客户端访问服务器删除信息                 |
| http_client_conn   | HTTP组件用于客户端连接服务器                         |
| http_client_send   | HTTP组件用于客户端发送消息给服务器                   |
| http_client_recv   | HTTP组件用于客户端接收服务器接受的信息               |

## API说明

## http_client_get

该函数用于下载服务器页面或者文件，其函数原型如下：

```c
HTTP_RESULT_CODE http_client_get(http_client_t *client, const char *url, http_client_data_t *client_data);
```

| **参数**         | **说明**                                       |
| :--------------- | :--------------------------------------------- |
| client           | 存放访问服务器的客户端信息结构体               |
| url              | 服务器url地址                                  |
| client_data      | HTTP用于数据的发送和接受服务器相关的数据结构体 |
| **返回**         | **说明**                                       |
| HTTP_RESULT_CODE | 错误码                                         |

## http_client_post

该函数用于访问服务器上传信息，其函数原型如下：

```c
HTTP_RESULT_CODE http_client_post(http_client_t *client, const char *url, http_client_data_t *client_data);
```

| **参数**         | **说明**                                       |
| :--------------- | :--------------------------------------------- |
| client           | 存放访问服务器的客户端信息结构体               |
| url              | 服务器url地址                                  |
| client_data      | HTTP用于数据的发送和接受服务器相关的数据结构体 |
| **返回**         | **说明**                                       |
| HTTP_RESULT_CODE | 错误码                                         |

## http_client_put

该函数用于访问服务器制定位置上传信息，其函数原型如下：

```c
HTTP_RESULT_CODE http_client_put(http_client_t *client, const char *url, http_client_data_t *client_data);
```

| **参数**         | **说明**                                       |
| :--------------- | :--------------------------------------------- |
| client           | 存放访问服务器的客户端信息结构体               |
| url              | 服务器url地址                                  |
| client_data      | HTTP用于数据的发送和接受服务器相关的数据结构体 |
| **返回**         | **说明**                                       |
| HTTP_RESULT_CODE | 错误码                                         |

## http_client_head

该函数用于访问服务器返回不带响应信息的消息，其函数原型如下：

```c
HTTP_RESULT_CODE http_client_head(http_client_t *client, const char *url, http_client_data_t *client_data);
```

| **参数**         | **说明**                                       |
| :--------------- | :--------------------------------------------- |
| client           | 存放访问服务器的客户端信息结构体               |
| url              | 服务器url地址                                  |
| client_data      | HTTP用于数据的发送和接受服务器相关的数据结构体 |
| **返回**         | **说明**                                       |
| HTTP_RESULT_CODE | 错误码                                         |

## http_client_delete

该函数用于访问服务器删除相应内容，其函数原型如下：

```c
HTTP_RESULT_CODE http_client_delete(http_client_t *client, const char *url, http_client_data_t *client_data);
```

| **参数**         | **说明**                                       |
| :--------------- | :--------------------------------------------- |
| client           | 存放访问服务器的客户端信息结构体               |
| url              | 服务器url地址                                  |
| client_data      | HTTP用于数据的发送和接受服务器相关的数据结构体 |
| **返回**         | **说明**                                       |
| HTTP_RESULT_CODE | 错误码                                         |

## http_client_conn

该函数用于连接服务器，其函数原型如下：

```c
HTTP_RESULT_CODE http_client_conn(http_client_t *client, const char *url);
```

| **参数**         | **说明**                         |
| :--------------- | :------------------------------- |
| client           | 存放访问服务器的客户端信息结构体 |
| url              | 服务器url地址                    |
| **返回**         | **说明**                         |
| HTTP_RESULT_CODE | 错误码                           |

## http_client_send

该函数用于发送消息给服务器，其函数原型如下：

```c
HTTP_RESULT_CODE http_client_send(http_client_t *client, const char *url, int method, http_client_data_t *client_data);
```

| **参数**         | **说明**                                       |
| :--------------- | :--------------------------------------------- |
| client           | 存放访问服务器的客户端信息结构体               |
| url              | 服务器url地址                                  |
| method           | 方法类型                                       |
| client_data      | HTTP用于数据的发送和接受服务器相关的数据结构体 |
| **返回**         | **说明**                                       |
| HTTP_RESULT_CODE | 错误码                                         |

## http_client_recv

该函数用于接收服务器发送的消息内容，其函数原型如下：

```c
HTTP_RESULT_CODE http_client_recv(http_client_t *client, http_client_data_t *client_data);
```

| **参数**         | **说明**                                       |
| :--------------- | :--------------------------------------------- |
| client           | 存放访问服务器的客户端信息结构体               |
| client_data      | HTTP用于数据的发送和接受服务器相关的数据结构体 |
| **返回**         | **说明**                                       |
| HTTP_RESULT_CODE | 错误码                                         |

## HTTP_RESULT_CODE 错误码说明

| **返回**            | **说明**                 |
| :------------------ | :----------------------- |
| HTTP_RESULT_CODE    | 错误码                   |
| HTTP_EAGAIN = 1     | 有更多的数据需要检索接收 |
| HTTP_SUCCESS  = 0   | 数据处理成功正常关闭     |
| HTTP_ENOBUFS = -1   | 缓存错误                 |
| HTTP_EARG     = -2  | 不合法参数               |
| HTTP_ENOTSUPP = -3  | 不支持                   |
| HTTP_EDNS     = -4  | DNS解析失败              |
| HTTP_ECONN    = -5  | 连接失败                 |
| HTTP_ESEND    = -6  | 发送包失败               |
| HTTP_ECLSD    = -7  | 连接关闭                 |
| HTTP_ERECV    = -8  | 接收数据包失败           |
| HTTP_EPARSE   = -9  | url地址解析失败          |
| HTTP_EPROTO   = -10 | 协议错误                 |
| HTTP_EUNKOWN  = -11 | 未知错误                 |
| HTTP_ETIMEOUT = -12 | 超时                     |
