# OneNET-NBIoT物联网套件

## 简介

OneNET推出的NB-IoT物联网套件为用户提供“终端-平台-应用”整体解决方案，帮助企业快速实现NB-IoT能力升级。

主要面向基于蜂窝的窄带物联网（Narrow Band Internet of Things, NB-IoT）场景下物联网应用，聚焦于低功耗广覆盖（LPWA）物联网市场。具有覆盖广、连接多、速率低、成本低、功耗低、架构优等特点。

基于NB-IOT的LwM2M协议和CoAP协议实现UE与OneNET平台的通信，其中实现数据传输的协议为CoAP，应用层协议由LwM2M实现。

Coap协议有以下特点：

- 基于轻量级的UDP协议之上，具有重传机制
- 协议支持IP多播
- 协议包头小，仅为4个字节
- 功耗低，适用于低功耗物联网场景

LwM2M属于轻量级的协议，适用于各种物联网设备，LwM2M定义了三个逻辑实体：

- LwM2M Server 服务器
- LwM2M Client 客户端，负责执行服务器的命令和上报执行结果
- LwM2M 引导服务器 Bootstrap Server，负责配置LwM2M客户端

## 重要定义及数据结构

### 用户配置结构体

用于配置网络参数，其定义如下：

```c
typedef struct
{
    uint8_t     bs_enabled;
    uint8_t     dtls_enabled;
    uint8_t    *ip;
    uint8_t    *port;
    uint8_t    *auth_code;
    uint8_t    *psk;
}cis_user_cfg_t;
```

| 重要成员     | 说明         |
| ------------ | ------------ |
| bs_enabled   | BS流程开关   |
| dtls_enabled | DTLS功能开关 |
| ip           | 服务器ip地址 |
| port         | 服务器端口号 |
| auth_code    | 校验码       |
| psk          | 共享密钥     |

### 挂接回调函数结构体

在cis连接实例初始化时需要传入回调函数的结构体，其定义如下：

```c
struct st_cis_callback
{
    cis_read_callback_t         onRead;
    cis_write_callback_t        onWrite;
    cis_exec_callback_t         onExec;
    cis_observe_callback_t      onObserve;
	cis_discover_callback_t     onDiscover;
	cis_set_params_callback_t   onSetParams;
    cis_event_callback_t        onEvent;
};
typedef struct st_cis_callback cis_callback_t;
```

| 重要成员    | 说明                 |
| ----------- | -------------------- |
| onRead      | 读操作回调函数       |
| onWrite     | 写操作回调函数       |
| onExec      | 执行操作回调函数     |
| onObserve   | 对象订阅操作回调函数 |
| onDiscover  | 资源发现操作回调函数 |
| onSetParams | 参数设置操作回调函数 |
| onEvent     | 事件回调函数         |

回调函数的定义常见API列表。

### 对象实例结构体

用于标识实例数量及是否对平台开放订阅，其定义如下：

```c
struct st_cis_inst_bitmap
{
    cis_instcount_t  instanceCount;
    cis_instcount_t  instanceBytes;
    const uint8_t   *instanceBitmap;
};
typedef struct st_cis_inst_bitmap cis_inst_bitmap_t;
```

| 重要成员       | 说明                                                         |
| -------------- | ------------------------------------------------------------ |
| instanceCount  | 对象实例数量                                                 |
| instanceBytes  | instanceBitmap字节长度                                       |
| instanceBitmap | 实例位图，1表示有效，0表示无效。实例位图可视为一个数组，数组第0个字节的最高位对应于第0个实例，以此类推 |

### 资源数量结构体

用于标识读写和执行资源数量，其定义如下：

```c
struct st_cis_res_count
{
    cis_attrcount_t attrCount;
    cis_actcount_t  actCount;
};
typedef struct st_cis_res_count		cis_res_count_t;
```

| 重要成员  | 说明               |
| --------- | ------------------ |
| attrCount | 可读写操作资源数量 |
| actCount  | 可执行操作资源数量 |

### URI结构体

用于表示对象、实例和资源的编号及有效标识，其定义如下：

```c
struct st_uri
{
	uint8_t      flag;           
	cis_oid_t    objectId;
	cis_iid_t    instanceId;
	cis_rid_t    resourceId;
};
typedef struct st_uri cis_uri_t;
```

| 重要成员   | 说明                           |
| ---------- | ------------------------------ |
| flag       | 有效标识，常见URI_FLAG_XXX定义 |
| objectId   | 对象ID                         |
| instanceId | 实例ID                         |
| resourceId | 资源ID                         |

### data数据结构体

在与平台交互data数据时使用的结构体，其定义如下：

```c
struct st_cis_data
{
    cis_rid_t               id;
    cis_datatype_t          type;

    struct
    {
        uint32_t    		length;
        uint8_t            *buffer;
    }asBuffer;

	union
	{
		bool            	asBoolean;
		int64_t         	asInteger;
		double          	asFloat;
	} value;
} ;
typedef struct st_cis_data cis_data_t;
```

| 重要成员 | 说明                             |
| -------- | -------------------------------- |
| id       | 资源ID                           |
| type     | 资源类型，常见cis_datatype_t定义 |
| asBuffer | 字符串数据                       |
| value    | bool、int、float数据             |

### 参数设置结构体

用于平台下发的参数设置信息，其定义如下：

```c
struct st_cis_observe_attr
{
    uint8_t     toSet;
    uint8_t     toClear;
    cis_time_t  minPeriod;
    cis_time_t  maxPeriod;
    float       greaterThan;
    float       lessThan;
    float       step;
};
typedef struct st_cis_observe_attr cis_observe_attr_t;
```

| 重要成员    | 说明                              |
| ----------- | --------------------------------- |
| toSet       | 设置标志位，参见ATTR_FLAG_XXX定义 |
| toClear     | 清除标志位                        |
| minPeriod   | 最小时间间隔，单位为秒            |
| maxPeriod   | 最大时间间隔                      |
| greaterThan | 观测数据大于该值时立即上报        |
| lessThan    | 观测数据小于该值时立即上报        |
| step        | 观测数据变化的最小区间            |



## API列表

| 接口                      | 说明                                 |
| ------------------------- | ------------------------------------ |
| cis_init                  | cis连接实例初始化                    |
| cis_deinit                | cis连接实例去初始化                  |
| cis_register              | 向OneNET平台注册cis连接实例          |
| cis_unregister            | 向OneNET平台注销连接                 |
| cis_addobject             | 添加object                           |
| cis_delobject             | 删除object                           |
| cis_pump                  | cis模块主调度接口                    |
| cis_update_reg            | 更新注册信息，包含心跳或者object信息 |
| cis_response              | 向平台回复操作结果                   |
| cis_notify                | 向平台上报数据                       |
| cis_uri_make              | 创建URI结构体                        |
| cis_read_callback_t       | 请求读数据操作回调函数               |
| cis_write_callback_t      | 请求写数据操作回调函数               |
| cis_exec_callback_t       | 请求执行操作回调函数                 |
| cis_observe_callback_t    | 请求观测操作回调函数                 |
| cis_discover_callback_t   | 请求发现操作回调函数                 |
| cis_set_params_callback_t | 请求参数设置操作回调函数             |
| cis_event_callback_t      | 事件回调函数                         |

### cis_init

该函数用于cis连接实例初始化，函数原型如下：

```c
cis_ret_t cis_init(void **context, cis_user_cfg_t *cis_user_cfg, void *DMconf);
```

| 参数         | 说明                                         |
| ------------ | -------------------------------------------- |
| context      | cis上下文                                    |
| cis_user_cfg | 用户初始化配置项                             |
| DMconf       | 设备管理配置，非DM时传NULL，当前不支持DM功能 |
| **返回**     | **说明**                                     |
| 非0          | 操作失败                                     |
| 0            | 操作成功                                     |

### cis_deinit

该函数用于cis连接实例去初始化，函数原型如下：

```c
cis_ret_t cis_deinit(void **context);
```

| 参数     | 说明      |
| -------- | --------- |
| context  | cis上下文 |
| **返回** | **说明**  |
| 非0      | 操作失败  |
| 0        | 操作成功  |

### cis_register

该函数用于cis实例初始化完成后，该接口向平台进行注册，函数原型如下：

```c
cis_ret_t cis_register(void *context, cis_time_t lifetime, const cis_callback_t *cb);
```

| 参数     | 说明             |
| -------- | ---------------- |
| context  | cis上下文        |
| lifetime | 保活周期         |
| cb       | 回调接口的结构体 |
| **返回** | **说明**         |
| 非0      | 操作失败         |
| 0        | 操作成功         |

### cis_unregister

该函数用于向服务器发起注销操作，函数原型如下：

```c
cis_ret_t cis_unregister(void *context);
```

| 参数     | 说明      |
| -------- | --------- |
| context  | cis上下文 |
| **返回** | **说明**  |
| 非0      | 操作失败  |
| 0        | 操作成功  |

### cis_addobject

该接口用于添加object信息，函数原型如下：

```c
cis_ret_t cis_addobject(void                     *context, 
                        cis_oid_t                 objectid, 
                        const cis_inst_bitmap_t  *bitmap，
                        const cis_res_count_t    *rescount);
```

| 参数     | 说明                                           |
| -------- | ---------------------------------------------- |
| context  | cis上下文                                      |
| objectid | object 编号                                    |
| bitmap   | 每一位表示一个实例，有效地实例为 1，无效的为 0 |
| rescount | 资源的可读写，可执行数量                       |
| **返回** | **说明**                                       |
| 非0      | 操作失败                                       |
| 0        | 操作成功                                       |

### cis_delobject

该接口用于删除object信息，函数原型如下：

```c
cis_ret_t cis_delobject(void * context, cis_oid_t objectid);
```

| 参数     | 说明        |
| -------- | ----------- |
| context  | cis上下文   |
| objectid | object 编号 |
| **返回** | **说明**    |
| 非0      | 操作失败    |
| 0        | 操作成功    |

### cis_pump

该接口用于在初始化完成后，调用该接口驱动cis实例运行，基础通信套件在该接口中调度多个任务，包括根据网络回复和状态发起注册消息，根据设定的 lifetime 定期发起更新注册消息等。函数原型如下：

```c
uint32_t cis_pump(void *context);
```

| 参数     | 说明         |
| -------- | ------------ |
| context  | cis上下文    |
| **返回** | **说明**     |
| -1       | 无待处理任务 |
| 0        | 有待处理任务 |

### cis_update_reg

该接口用于通知平台更新注册的 lifetime。如果withObjects  参数为 true，则同时更新 object 信息，如果 withObjects 参数为 false，则只更新 lifetime。函数原型如下：

```c
uint32_t cis_update_reg(void *context, uint32_t lifetime, bool withObjects);
```

| 参数        | 说明               |
| ----------- | ------------------ |
| context     | cis上下文          |
| lifetime    | 保活周期           |
| withObjects | 是否更新object信息 |
| **返回**    | **说明**           |
| -1          | 操作失败           |
| 0           | 操作成功           |

### cis_response

当网络侧发来读、写、执行、观测、设置参数等请求时，CIS实例会调用相关回调函数，并在回调中携带相应msgid，待操作完成时，应用程序使用 cis_response 接口返回结果。如果操作成功：对于读操作，如果是这组读操作的最后一条返回，则result设为 CIS_RESPONSE_READ，如果不是最后一条消息，result 设置为 CIS_RESPONSE_CONTINUE。uri是读取的当前数据的标识，msgid 是该条请求的标识，使用对应 callback 携带的 msgid；对于写、执行、观测、设置参数，分别将操作结果 result 设置为 CIS_RESPONSE_WRITE 、 CIS_RESPONSE_EXECUTE 、CIS_RESPONSE_OBSERVE、  CIS_RESPONSE_OBSERVE_PARAMS，uri 和 value 为 null。如果请求的操作失败，包括读、写、执行、观测、设置参数，则将 uri 和 value 置为空，result 设置为对应的错误结果。函数原型如下：

```c
cis_ret_t cis_response (void             *context,
                        const cis_uri_t  *uri, 
                        const cis_data_t *value,
                        cis_mid_t         mid, 
                        cis_coapret_t     result);
```

| 参数     | 说明                  |
| -------- | --------------------- |
| context  | cis上下文             |
| uri      | 该条操作的值对应的uri |
| value    | 上报的数据            |
| mid      | 需要回复的消息ID      |
| result   | 操作结果              |
| **返回** | **说明**              |
| 非0      | 操作失败              |
| 0        | 操作成功              |

### cis_notify

应用程序根据网络侧所设置的数据上报策略调用cis_notify接口进行数据上报，一组上报操作最多可以上报一个instance 的数据，如果需要上报多个 instance 的数据，需要将数据分为多组进行上报。另外，每一次调用上报一个 resource 的数据，用户需要提供对应observe消息的msgid和该resouce 对应的uri，对于一组上报，如果不是最后一条消息，则 result 字 段 应 为CIS_NOTIFY_CONTINUE，如果是最后一条消息，result 应该为CIS_NOTIFY_CONTENT。requestACK为保留参数，表示该组上报消息是否需要以 CON 形式上报。函数原型如下：

```c
cis_ret_t cis_notify(void              *context, 
                     const cis_uri_t   *uri, 
                     const cis_data_t  *value,
                     cis_mid_t          mid, 
                     cis_coapret_t      result, 
                     bool               needAck);
```

| 参数     | 说明                  |
| -------- | --------------------- |
| context  | cis上下文             |
| uri      | 该条操作的值对应的uri |
| value    | 上报的数据            |
| mid      | 需要回复的消息ID      |
| result   | 操作结果              |
| needAck  | 是否要求ack           |
| **返回** | **说明**              |
| 非0      | 操作失败              |
| 0        | 操作成功              |

### cis_uri_make

使用该接口生成标准uri结构体，函数原型如下：

```c
cis_ret_t cis_uri_make(cis_oid_t  oid, 
                       cis_iid_t  iid, 
                       cis_rid_t  rid, 
                       cis_uri_t *uri);
```

| 参数     | 说明        |
| -------- | ----------- |
| oid      | 对象id      |
| iid      | 实例id      |
| rid      | 资源id      |
| uri      | 待生成的uri |
| **返回** | **说明**    |
| -1       | 操作失败    |
| 0        | 操作成功    |

### cis_read_callback_t

读操作回调函数，函数原型如下：

```c
typedef cis_coapret_t(*cis_read_callback_t) (void        *context,
                                             cis_uri_t   *uri,
                                             cis_mid_t    mid);
```

| 参数     | 说明      |
| -------- | --------- |
| context  | cis上下文 |
| uri      | 实例id    |
| mid      | 消息id    |
| **返回** | **说明**  |
| 非0      | 操作失败  |
| 0        | 操作成功  |

### cis_write_callback_t

写操作回调函数，函数原型如下：

```c
typedef cis_coapret_t (*cis_write_callback_t)(void               *context,
                                              cis_uri_t          *uri,
                                              const cis_data_t   *value,
                                              cis_attrcount_t     attrcount,
                                              cis_mid_t           mid);
```

| 参数      | 说明                            |
| --------- | ------------------------------- |
| context   | cis上下文                       |
| uri       | 实例id                          |
| value     | 平台下发的写入数据数组          |
| attrcount | 资源数量，即是value数组元素数量 |
| mid       | 消息id                          |
| **返回**  | **说明**                        |
| 非0       | 操作失败                        |
| 0         | 操作成功                        |

### cis_exec_callback_t

执行操作回调函数，函数原型如下：

```c
typedef cis_coapret_t (*cis_exec_callback_t)(void           *context,
                                             cis_uri_t      *uri,
                                             const uint8_t  *buffer,
                                             uint32_t        length,
                                             cis_mid_t       mid);
```

| 参数     | 说明                   |
| -------- | ---------------------- |
| context  | cis上下文              |
| uri      | 实例id                 |
| buffer   | 平台下发的执行操作命令 |
| length   | buffer长度             |
| mid      | 消息id                 |
| **返回** | **说明**               |
| 非0      | 操作失败               |
| 0        | 操作成功               |

### cis_observe_callback_t

对象订阅操作回调函数，函数原型如下：

```c
typedef cis_coapret_t (*cis_observe_callback_t)(void           *context,
                                                cis_uri_t      *uri,
                                                bool            flag,
                                                cis_mid_t       mid);
```

| 参数     | 说明                 |
| -------- | -------------------- |
| context  | cis上下文            |
| uri      | 实例id               |
| flag     | 订阅或者取消订阅标识 |
| mid      | 消息id               |
| **返回** | **说明**             |
| 非0      | 操作失败             |
| 0        | 操作成功             |

### cis_discover_callback_t

资源发现操作回调函数，函数原型如下：

```c
typedef cis_coapret_t (*cis_discover_callback_t)(void       *context,
                                                 cis_uri_t  *uri,
                                                 cis_mid_t   mid);
```

| 参数     | 说明      |
| -------- | --------- |
| context  | cis上下文 |
| uri      | 实例id    |
| mid      | 消息id    |
| **返回** | **说明**  |
| 非0      | 操作失败  |
| 0        | 操作成功  |

### cis_set_params_callback_t

设置参数操作回调函数，函数原型如下：

```c
typedef cis_coapret_t (*cis_set_params_callback_t)(void                 *context,
                                                   cis_uri_t            *uri,
                                                   cis_observe_attr_t    parameters,
                                                   cis_mid_t             mid);
```

| 参数       | 说明                 |
| ---------- | -------------------- |
| context    | cis上下文            |
| uri        | 实例id               |
| parameters | 平台下发的参数结构体 |
| mid        | 消息id               |
| **返回**   | **说明**             |
| 非0        | 操作失败             |
| 0          | 操作成功             |

### cis_event_callback_t

事件回调函数，函数原型如下：

```c
typedef void (*cis_event_callback_t)(void *context, cis_evt_t id, void *param);
```

| 参数     | 说明      |
| -------- | --------- |
| context  | cis上下文 |
| id       | 事件id    |
| param    | 事件参数  |
| **返回** | **说明**  |
| 无       | 无        |

