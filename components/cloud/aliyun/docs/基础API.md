# 基础API



##### IOT_OpenLog

日志系统的初始化函数, 本接口被调用后, SDK才有可能向终端打印日志文本, 但打印的文本详细程度还是由 `IOT_SetLogLevel()` 确定, 默认情况下, 无日志输出

```c
void IOT_OpenLog(const char *ident);
```

| 参数     | 说明                                               |
| -------- | -------------------------------------------------- |
| ident    | 日志的标识符字符串, 例如: `IOT_OpenLog("linkkit")` |
| **返回** |                                                    |
| 无返回值 |                                                    |



##### IOT_CloseLog

日志系统的销毁函数, 本接口被调用后, SDK停止向终端打印任何日志文本, 但之后可以由调用 `IOT_OpenLog()` 接口重新使能日志输出

关闭和重新使能日志系统之后, 需要重新调用 `IOT_SetLogLevel()` 接口设置日志级别, 否则日志系统虽然使能了, 也不会输出文本

```c
void IOT_CloseLog(void);
```

| 参数       | 说明 |
| ---------- | ---- |
| 无参数     |      |
| **返回值** |      |
| 无返回值   |      |



##### IOT_SetLogLevel

日志系统的日志级别配置函数, 本接口用于设置SDK的日志显示级别, 需要在 `IOT_OpenLog()` 后被调用

```c
void IOT_SetLogLevel(IOT_LogLevel level);
```

| 参数       | 说明               |
| ---------- | ------------------ |
| level      | 需要显示的日志级别 |
| **返回值** |                    |
|            |                    |

###### 附加参数说明

```c
typedef enum _IOT_LogLevel {
    IOT_LOG_EMERG = 0,
    IOT_LOG_CRIT,
    IOT_LOG_ERROR,
    IOT_LOG_WARNING,
    IOT_LOG_INFO,
    IOT_LOG_DEBUG,
} IOT_LogLevel;
```



##### IOT_DumpMemoryStats

该接口可显示出SDK各模块的内存使用情况, 当WITH_MEM_STATS=1时起效, 显示级别设置得越高, 显示的信息越多

```c
void IOT_DumpMemorStats(IOT_LogLevel level);
```

| 参数       |                    |
| ---------- | ------------------ |
| leve       | 需要显示的日志级别 |
| **返回值** |                    |
| 无返回值   |                    |



##### IOT_SetupConnInfo

在连接云端之前, 需要做一些认证流程, 如一型一密获取DeviceSecret或者根据当前所选认证模式向云端进行认证

该接口在SDK基础版中需要在连接云端之前由用户显式调用. 而在高级版中SDK会自动进行调用, 不需要用户显式调用

```c
int IOT_SetupConnInfo(const char *product_key,
                        const char *device_name,
                        const char *device_secret,
                        void **info_ptr);
```

| 参数          | 说明                                                         |
| ------------- | ------------------------------------------------------------ |
| product_key   | 设备三元组的ProductKey                                       |
| device_name   | 设备三元组的DeviceName                                       |
| device_secret | 设备三元组的DeviceSecret                                     |
| info_ptr      | 该void**数据类型为iotx_conn_info_t, 在认证流程通过后, 会得到云端的相关信息, 用于建立与云端连接时使用 |
| **返回值**    |                                                              |
| 0             | 成功                                                         |
| <0            | 失败                                                         |

###### 参数附加说明

```c
typedef struct {
    uint16_t        port;
    char            host_name[HOST_ADDRESS_LEN + 1];
    char            client_id[CLIENT_ID_LEN + 1];
    char            username[USER_NAME_LEN + 1];
    char            password[PASSWORD_LEN + 1];
    const char     *pub_key;
} iotx_conn_info_t, *iotx_conn_info_pt;
```



##### IOT_Ioctl

在SDK连接云端之前, 用户可用此接口进行SDK部分参数的配置或获取, 如连接的region, 是否使用一型一密等

该接口在基础版和高级版中均适用, 需要注意的是, 该接口需要在SDK建立网络连接之前调用. 关于一型一密的

```c
int IOT_Ioctl(int option, void *data);
```

| 参数       | 说明                                             |
| ---------- | ------------------------------------------------ |
| option     | 选择需要配置或获取的参数                         |
| data       | 在配置或获取参数时需要的buffer, 依据`option`而定 |
| **返回值** |                                                  |
| 0          | 成功                                             |
| <0         | 失败                                             |

###### 附加参数说明

```c
typedef enum {
    IOTX_IOCTL_SET_DOMAIN,              /* value(int*): iotx_cloud_domain_types_t */
    IOTX_IOCTL_GET_DOMAIN,              /* value(int*) */
    IOTX_IOCTL_SET_DYNAMIC_REGISTER,    /* value(int*): 0 - Disable Dynamic Register, 1 - Enable Dynamic Register */
    IOTX_IOCTL_GET_DYNAMIC_REGISTER     /* value(int*) */
} iotx_ioctl_option_t;
```

