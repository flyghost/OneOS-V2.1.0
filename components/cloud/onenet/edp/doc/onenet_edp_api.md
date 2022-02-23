# OneNET EDP接入协议

------

## 简介

EDP (Enhanced Device Protocol增强设备协议）是OneNET平台根据物联网特点专门定制的完全公开的基于TCP的协议，可以广泛应用到家居、交通、物流、能源以及其他行业应用中。

具有以下功能特点：

1. 长连接协议；
2. 数据加密传输；
3. 终端数据点上报，支持的数据点类型为：浮点数(float)、整型（int）、字符串、JSON对象/二进制数据；
4. 平台消息下发（支持离线消息）；
5. 端到端数据转发。

------

## 重要定义及数据结构

### 消息类型

平台与设备之间的交互消息类型定义如下：

| **消息类型** | **值** | **流向** | **描述**                   |
| ------------ | ------ | -------- | -------------------------- |
| CONN_REQ     | 1      | C->S     | 客户端请求与服务端建立连接 |
| CONN_RESP    | 2      | S->C     | 服务端确认连接建立         |
| PUSH_DATA    | 3      | C<->S    | 转发（透传）数据           |
| CONN_CLOSE   | 4      | S->C     | 连接关闭                   |
| UPDATE_REQ   | 5      | C->S     | 上报当前使用的软件信息     |
| UPDATE_RESP  | 6      | S->C     | 平台下发当前最新的软件信息 |
| SAVE_DATA    | 8      | C<->S    | 存储（&转发）数据          |
| SAVE_ACK     | 9      | S->C     | 存储确认                   |
| CMD_REQ      | 10     | S->C     | 命令请求                   |
| CMD_RESP     | 11     | C->S     | 命令回复                   |
| PING_REQ     | 12     | C->S     | 心跳请求                   |
| PING_RESP    | 13     | S->C     | 心跳响应                   |
| ENCRYPT_REQ  | 14     | C->S     | 加密请求                   |
| ENCRYPT_RESP | 15     | S->C     | 加密回复                   |

## API列表

| **接口**           | **说明**       |
| :----------------- | :------------- |
| PacketConnect1     | 连接OneNET平台 |
| PacketConnect2     | 连接OneNET平台 |
| GetEdpPacket       | 接收EDP数据包  |
| EdpPacketType      | 获取EDP包类型  |
| PacketPushdata     | 转发数据       |
| UnpackPushdata     | 解析转发数据   |
| PacketSavedataJson | 打包存储数据   |
| UnpackSavedataJson | 解析存储数据   |
| PacketPing         | 打包心跳包     |

## PacketConnect1

该函数用于打包由设备到设备云的EDP协议包，连接设备云的请求(登录认证方式1)，其函数原型如下:

```c
EdpPacket *PacketConnect1(const char *devid, const char *auth_key);
```

| **参数**   | **说明**                       |
| :--------- | :----------------------------- |
| devid      | 设备ID，申请设备时平台返回的ID |
| auth_key   | 鉴权信息(api-key)，从平台申请  |
| **返回**   | **说明**                       |
| EdpPacket* | EDP协议包                      |

说明：返回的EDP包发送给设备云后，需要客户程序删除该包。

## PacketConnect2

该函数用于打包由设备到设备云的EDP协议包，连接设备云的请求(登录认证方式2)，其函数原型如下:

```c
EdpPacket *PacketConnect2(const char *userid, const char *auth_info);
```

| **参数**   | **说明**                                  |
| :--------- | :---------------------------------------- |
| userid     | 用户ID，在平台注册账号时平台返回的用户ID  |
| auth_info  | 鉴权信息(api-key)，从平台申请，具备唯一性 |
| **返回**   | **说明**                                  |
| EdpPacket* | EDP协议包                                 |

说明：返回的EDP包发送给设备云后，需要客户程序删除该包。

## GetEdpPacket

该函数用于将接收到的二进制流，分解成一个一个的EDP包，函数原型如下：

```c
EdpPacket *GetEdpPacket(RecvBuffer *buf);
```

| **参数**   | **说明**  |
| :--------- | :-------- |
| buf        | 接收缓存  |
| **返回**   | **说明**  |
| EdpPacket* | EDP协议包 |

说明：返回的EDP包使用后，需要客户程序删除该包。

## EdpPacketType

该函数用于获取一个EDP包的消息类型，客户程序根据消息类型做不同的处理，函数原型如下：

```c
uint8 EdpPacketType(EdpPacket *pkg);
```

| **参数** | **说明**  |
| :------- | :-------- |
| pkg      | EDP协议包 |
| **返回** | **说明**  |
| uint8    | 消息类型  |

## PacketPushdata

该函数用于打包设备到设备云的EDP协议包，设备与设备之间转发数据，其函数原型如下：

```c
EdpPacket *PacketPushdata(const char *dst_devid, const char *data, uint32 data_len);
```

| **参数**   | **说明**   |
| :--------- | :--------- |
| dst_devid  | 目的设备ID |
| data       | 数据       |
| data_len   | 数据长度   |
| **返回**   | **说明**   |
| EdpPacket* | EDP协议包  |

说明：返回的EDP包使用后，需要客户程序删除该包。

## UnpackPushdata

该函数用于解包由设备云到设备的EDP协议包，设备与设备之间转发数据，其函数原型如下：

```c
int32 UnpackPushdata(EdpPacket *pkg, char **src_devid, char **data, uint32 *data_len);
```

| **参数**  | **说明**                         |
| :-------- | :------------------------------- |
| pkg       | EDP包，必须是pushdata包          |
| src_devid | 源设备ID                         |
| data      | 数据                             |
| data_len  | 数据长度                         |
| **返回**  | **说明**                         |
| int32     | =0表示解析成功，<0则表示解析失败 |

说明：通过函数GetEdpPacket和EdpPacketType判断出是pushdata后，将整个响应EDP包作为参数，由该函数进行解析，返回的源设备ID(src_devid)和数据(data)都需要客户端释放。

## PacketSavedataJson

该函数用于打包设备到设备云的EDP协议包，存储数据(json格式数据)，其函数原型如下：

```c
EdpPacket *PacketSavedataJson(const char *dst_devid, 
                              cJSON      *json_obj, 
                              int         type, 
                              uint16      msg_id);
```

| **参数**   | **说明**   |
| :--------- | :--------- |
| dst_devid  | 目的设备ID |
| json_obj   | json数据   |
| type       | json的类型 |
| msg_id     | 消息标志   |
| **返回**   | **说明**   |
| EdpPacket* | EDP协议包  |

说明：返回的EDP包发送给设备云后，需要客户程序删除该包。

## UnpackSavedataJson

该函数用于解包由设备云到设备的EDP协议包，存储数据(json格式数据)，其函数原型如下：

```c
int32 UnpackSavedataJson(EdpPacket *pkg, cJSON **json_obj);
```

| **参数** | **说明**                            |
| :------- | :---------------------------------- |
| pkg      | EDP包，必须是savedata包的json数据包 |
| json_obj | json数据                            |
| **返回** | **说明**                            |
| int32    | =0表示解析成功，<0则表示解析失败    |

说明：返回的json数据(json_obj)需要客户端释放。

## PacketPing

该函数用于打包由设备到设备云的EDP心跳包，该函数原型如下：

```c
EdpPacket *PacketPing(void);
```

| **参数**   | **说明**  |
| :--------- | :-------- |
| 无         | 无        |
| **返回**   | **说明**  |
| EdpPacket* | EDP协议包 |

说明：返回的EDP包发送给设备云后，需要客户程序删除该包。
