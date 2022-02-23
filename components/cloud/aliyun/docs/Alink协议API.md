# Alink协议API



##### IOT_Linkkit_Open

初始化设备资源, 在对设备进行操作之前, 必须先调用此接口. 该接口调用成功的情况下会返回设备ID, 当使用其他接口时需要以设备ID为入参, 对指定的设备进行操作

```c
int IOT_Linkkit_Open(iotx_linkkit_dev_type_t dev_type, iotx_linkkit_dev_meta_info_t *meta_info);
```

| 参数       | 说明                   |
| ---------- | ---------------------- |
| dev_type   | 需要创建资源的设备类型 |
| meta_info  | 设备的四元组信息       |
| **返回值** |                        |
| \>= 0      | 创建设备成功           |
| < 0        | 失败                   |

###### 参数附加说明

```c
typedef enum {
    IOTX_LINKKIT_DEV_TYPE_MASTER,
    IOTX_LINKKIT_DEV_TYPE_SLAVE,
    IOTX_LINKKIT_DEV_TYPE_MAX
} iotx_linkkit_dev_type_t;
```

- IOTX_LINKKIT_DEV_TYPE_MASTER: 创建的设备为主设备, 仅能创建一次

- IOTX_LINKKIT_DEV_TYPE_SLAVE: 创建的设备为子设备

  

```c
typedef struct {
    char product_key[PRODUCT_KEY_MAXLEN];
    char product_secret[PRODUCT_SECRET_MAXLEN];
    char device_name[DEVICE_NAME_MAXLEN];
    char device_secret[DEVICE_SECRET_MAXLEN];
} iotx_linkkit_dev_meta_info_t;
```

- product_key: 最大长度为20字节
- product_secret: 最大长度为64字节
- device_name: 最大长度为32字节
- device_secret: 最大长度为64字节



##### IOT_Linkkit_Connect

对于主设备来说, 将会建立设备与云端的通信. 对于子设备来说, 将向云端注册该子设备(如果需要的话), 并添加主子设备拓扑关系

```c
int IOT_Linkkit_Connect(int devid);
```

| 参数   | 说明   |
| ------ | ------ |
| devid  | 设备ID |
| 返回值 |        |
| 0      | 成功   |
| < 0    | 失败   |



##### IOT_Linkkit_Yield

若SDK占有独立线程, 该函数只将接收到的网络报文分发到用户的回调函数中, 否则表示将CPU交给SDK让其接收网络报文并将消息分发到用户的回调函数中

```c
void IOT_Linkkit_Yield(int timeout_ms);
```

| 参数       | 说明                                         |
| ---------- | -------------------------------------------- |
| timeout_ms | 单线程模式下, 每次尝试接收网络报文的超时时间 |
| **返回值** |                                              |
| 无返回值   |                                              |



##### IOT_Linkkit_Close

若设备ID为主设备, 则关闭网络连接并释放Linkkit所有占用资源

```c
int IOT_Linkkit_Close(int devid);
```

| 参数       | 说明   |
| ---------- | ------ |
| devid      | 设备ID |
| **返回值** |        |
| 0          | 成功   |
| < 0        | 失败   |



##### IOT_Linkkit_TriggerEvent

向云端上报设备事件

```c
int IOT_Linkkit_TriggerEvent(int devid, char *eventid, int eventid_len, char *payload, int payload_len);
```

| 参数        | 说明              |
| ----------- | ----------------- |
| devid       | 设备ID            |
| eventid     | TSL中定义的事件ID |
| eventid_len | 事件ID的长度      |
| payload     | 事件Payload       |
| payload_len | 事件Payload的长度 |
| **返回值**  |                   |
| \>= 1       | 消息ID            |
| < 0         | 失败              |



##### IOT_Linkkit_Report

向云端发送消息, 包括属性上报/设备标签信息更新上报/设备标签信息删除上报/透传数据上报/子设备登录/子设备登出

```c
int IOT_Linkkit_Report(int devid,
                        iotx_linkkit_msg_type_t msg_type,
                        unsigned char *payload,
                        int payload_len);
```

| 参数        | 说明               |
| ----------- | ------------------ |
| devid       | 设备ID             |
| msg_type    | 需要上报的消息类型 |
| payload     | 消息Payload        |
| payload_len | 消息Payload的长度  |
| **返回值**  |                    |
| \>= 1       | 消息ID             |
| < 0         | 失败               |
| 0           | 成功               |

参数附加说明

```c
typedef enum {
    ITM_MSG_POST_PROPERTY,
    ITM_MSG_DEVICEINFO_UPDATE,
    ITM_MSG_DEVICEINFO_DELETE,
    ITM_MSG_POST_RAW_DATA,
    ITM_MSG_LOGIN,
    ITM_MSG_LOGOUT,
    ...
    ...

    IOTX_LINKKIT_MSG_MAX
} iotx_linkkit_msg_type_t;
```

- ITM_MSG_POST_PROPERTY: 设备属性数据上报
- ITM_MSG_DEVICEINFO_UPDATE: 设备标签更新信息上报
- ITM_MSG_DEVICEINFO_DELETE: 设备标签删除信息上报
- ITM_MSG_POST_RAW_DATA: 设备透传数据上报
- ITM_MSG_LOGIN: 子设备登录
- ITM_MSG_LOGOUT: 子设备登出
- ITM_MSG_DELETE_TOPO: 删除子设备和网关之间的拓扑关系
- ITM_MSG_REPORT_SUBDEV_FIRMWARE_VERSION: 上报子设备的固件版本号, 用于子设备OTA功能
- ITM_MSG_PROPERTY_DESIRED_GET: 获取云端缓存的属性值下发, 用于高级版设备影子
- ITM_MSG_PROPERTY_DESIRED_DELETE: 主动删除云端缓存的属性值, 用于高级版设备影子



##### IOT_Linkkit_Query

向云端查询数据, 包括查询时间戳/查询设备的拓扑关系列表/查询FOTA升级的固件数据/查询COTA升级的Config文件数据/查询是否有可用的新固件/查询是否有可用的Config文件

```c
int IOT_Linkkit_Query(int devid,
                        iotx_linkkit_msg_type_t msg_type,
                        unsigned char *payload,
                        int payload_len);
```

| 参数        | 说明               |
| ----------- | ------------------ |
| devid       | 设备ID             |
| msg_type    | 需要上报的消息类型 |
| payload     | 消息Payload        |
| payload_len | 消息Payload的长度  |
| **返回值**  |                    |
| \>= 1       | 消息ID             |
| < 0         | 失败               |
| 0           | 成功               |

参数附加说明

```c
typedef enum {
    ...
    ...
    ITM_MSG_QUERY_TIMESTAMP,
    ITM_MSG_QUERY_TOPOLIST,
    ITM_MSG_QUERY_FOTA_DATA,
    ITM_MSG_QUERY_COTA_DATA,
    ITM_MSG_REQUEST_COTA,
    ITM_MSG_REQUEST_FOTA_IMAGE,

    IOTX_LINKKIT_MSG_MAX
} iotx_linkkit_msg_type_t;
```

- ITM_MSG_QUERY_TIMESTAMP: 查询时间戳
- ITM_MSG_QUERY_TOPOLIST: 查询设备的拓扑关系列表
- ITM_MSG_QUERY_FOTA_DATA: 查询FOTA升级的固件数据
- ITM_MSG_QUERY_COTA_DATA: 查询COTA升级的远程配置数据
- ITM_MSG_REQUEST_FOTA_IMAGE: 查询是否有可用的新固件
- ITM_MSG_REQUEST_COTA: 查询是否有可用的远程配置数据