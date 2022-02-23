# **OneTLS**安全通信组件

## 简介

OneTLS是一套轻量级的SSL/TLS库，支持最新的TLS1.3和DTLS1.3标准(draft 38)，向用户提供了简单易用的操作接口，为物联网通信安全提供了保障。

## 目录结构

| 目录   | 说明            |
| ------ | --------------- |
| crypto | 密码库          |
| tls    | tls和dtls源文件 |
| tets   | 测试示例        |

## OneTLS 公共API

### onetls_init

该函数用于初始化协议栈，其函数原型如下：

```c
uint32_t onetls_init(void);
```

| 参数     | 说明     |
| -------- | -------- |
| 无       | 无       |
| **返回** | **说明** |
| 0        | 成功     |
| 非0      | 失败     |

### onetls_version

该函数用于获取协议栈版本，其函数原型如下：

```c
const char *onetls_version(onetls_version_vrcb *vrcb);
```

| 参数     | 说明         |
| -------- | ------------ |
| 无       | 无           |
| **返回** | **说明**     |
| 字符串   | onetls版本号 |

### onetls_new_ctx

该函数用于新建会话管理上下文，其函数原型如下：

```c
onetls_ctx *onetls_new_ctx(uint8_t mode);
```

| 参数       | 说明             |
| ---------- | ---------------- |
| mode       | 安全通信协议模式 |
| **返回**   | **说明**         |
| onetls_ctx | 会话管理对象指针 |

### onetls_del_ctx

该函数用于释放会话管理上下文，其函数原型如下：

```c
void onetls_del_ctx(onetls_ctx *ctx);
```

| 参数     | 说明             |
| -------- | ---------------- |
| ctx      | 会话管理对象指针 |
| **返回** | **说明**         |
| 无       | 无               |

### onetls_set_socket

该函数用于绑定socket句柄到会话管理对象，其函数原型如下：

```c
void onetls_set_socket(onetls_ctx *ctx, int recv_fd, int send_fd);
```

| 参数     | 说明             |
| -------- | ---------------- |
| ctx      | 会话管理对象指针 |
| recv_fd  | 接收句柄         |
| send_fd  | 发送句柄         |
| **返回** | **说明**         |
| 无       | 无               |

### onetls_set_outband_psk_hint

该函数用于配置带外psk，其函数原型如下：

```c
uint32_t onetls_set_outband_psk_hint(onetls_ctx *ctx, uint8_t *hint, uint32_t hint_len);
```

| 参数     | 说明             |
| -------- | ---------------- |
| ctx      | 会话管理对象指针 |
| hint     | psk identity     |
| hint_len | psk identity长度 |
| **返回** | **说明**         |
| 0        | 成功             |
| 非0      | 失败             |

### onetls_connect

该函数用于发起握手连接，其函数原型如下：

```c
uint32_t onetls_connect(onetls_ctx *ctx);
```

| 参数     | 说明             |
| -------- | ---------------- |
| ctx      | 会话管理对象指针 |
| **返回** | **说明**         |
| 0        | 成功             |
| 非0      | 失败             |

### onetls_shutdown

该函数用于断开握手连接，其函数原型如下：

```c
uint32_t onetls_shutdown(onetls_ctx *ctx);
```

| 参数     | 说明             |
| -------- | ---------------- |
| ctx      | 会话管理对象指针 |
| **返回** | **说明**         |
| 0        | 成功             |
| 非0      | 失败             |

### onetls_send

该函数用于发送应用数据，其函数原型如下：

```c
uint32_t onetls_send(onetls_ctx *ctx, uint8_t *out, uint32_t out_len, uint32_t *send_len);
```

| 参数     | 说明               |
| -------- | ------------------ |
| ctx      | 会话管理对象指针   |
| out      | 待发送的数据缓存   |
| out_len  | 待发送的数据长度   |
| send_len | 实际发送的数据长度 |
| **返回** | **说明**           |
| 0        | 成功               |
| 非0      | 失败               |

### onetls_recv

该函数用于接收应用数据，其函数原型如下：

```c
uint32_t onetls_recv(onetls_ctx *ctx, uint8_t *in, uint32_t in_len, uint32_t *recv_len);
```

| 参数     | 说明               |
| -------- | ------------------ |
| ctx      | 会话管理对象指针   |
| in       | 接收数据缓存       |
| in_len   | 接收数据缓存长度   |
| recv_len | 实际接收的数据长度 |
| **返回** | **说明**           |
| 0        | 成功               |
| 非0      | 失败               |

### onetls_send_early_data

该函数用于发送早期应用数据，其函数原型如下：

```c
uint32_t onetls_send_early_data(onetls_ctx *ctx, uint8_t *out, uint32_t out_len);
```

| 参数     | 说明             |
| -------- | ---------------- |
| ctx      | 会话管理对象指针 |
| out      | 待发送的数据缓存 |
| out_len  | 待发送的数据长度 |
| **返回** | **说明**         |
| 0        | 成功             |
| 非0      | 失败             |

### onetls_flush

该函数用于清空缓冲区，其函数原型如下：

```c
uint32_t onetls_flush(onetls_ctx *ctx);
```

| 参数     | 说明             |
| -------- | ---------------- |
| ctx      | 会话管理对象指针 |
| **返回** | **说明**         |
| 0        | 成功             |
| 非0      | 失败             |

### onetls_pending

该函数用于检测缓冲区是否有数据，其函数原型如下：

```c
uint32_t onetls_pending(onetls_ctx *ctx);
```

| 参数     | 说明             |
| -------- | ---------------- |
| ctx      | 会话管理对象指针 |
| **返回** | **说明**         |
| 0        | 缓冲区空         |
| 非0      | 缓冲区非空       |

### onetls_set_nst_callback

该函数用于设置票据的接收回调，其函数原型如下：

```c
void onetls_set_nst_callback(onetls_ctx *ctx, onetls_nst_callback cb);
```

| 参数                     | 说明             |
| ------------------------ | ---------------- |
| ctx                      | 会话管理对象指针 |
| onetls_recv_nst_callback | 回调函数         |
| **返回**                 | **说明**         |
| 无                       | 无               |

### onetls_set_ticket

该函数用于设置票据参数，其函数原型如下：

```c
uint32_t onetls_set_ticket(onetls_ctx *ctx, void *data, uint32_t data_len);
```

| 参数     | 说明             |
| -------- | ---------------- |
| ctx      | 会话管理对象指针 |
| data     | 票据数据         |
| data_len | 票据数据长度     |
| **返回** | **说明**         |
| 0        | 成功             |
| 非0      | 失败             |

### onetls_update_key

该函数用于密钥更新，其函数原型如下：

```c
uint32_t onetls_update_key(onetls_ctx *ctx, void *data, uint8_t way);
```

| 参数     | 说明             |
| -------- | ---------------- |
| ctx      | 会话管理对象指针 |
| way      | 更新密钥的方式   |
| **返回** | **说明**         |
| 0        | 成功             |
| 非0      | 失败             |

### onetls_set_mtu

该函数用于设置MTU大小，其函数原型如下：

```c
uint32_t onetls_set_mtu(onetls_ctx *ctx, void *data, uint16_t mtu);
```

| 参数     | 说明             |
| -------- | ---------------- |
| ctx      | 会话管理对象指针 |
| mtu      | MTU大小          |
| **返回** | **说明**         |
| 0        | 成功             |
| 非0      | 失败             |

## 使用示例

参考tests目录下的tls和dtls测试示例

## 配置指南

### Menuconfig配置

在OneOS的SDK根目录下打开\projects\xxxxx文件夹，右键启动OneOS-Cube 工具，在命令行输入 menuconfig 打开可视化配置界面，选择"Components → Security → OneTLS"选项。其中第一项用于使能OneTLS安全通信协议栈，该组件使能后配置其他项。

### 使用GCC编译

基于上述配置，在OneOS-Cube 工具命令行输入 scons即用GCC进行编译，通过固件下载工具（如ST芯片较常使用的ST Utilities工具等）进行烧录。

### 使用ARM CC编译

基于上述配置，在OneOS-Cube 工具命令行输入 scons --ide=mdk5即可自动构建MDK工程，使用MDK5进行编译、烧录及调试。