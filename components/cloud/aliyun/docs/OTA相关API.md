# OTA相关API



##### IOT_OTA_Init

初始化OTA模块, 需要先建立与云端的MQTT连接后才能使用

```c
void *IOT_OTA_Init(const char *product_key, const char *device_name, void *ch_signal);
```

| 参数        | 说明                      |
| ----------- | ------------------------- |
| product_key | 设备三元组中的Product Key |
| device_name | 设备三元组中的Device Name |
| ch_signal   | MQTT句柄                  |
| **返回值**  |                           |
| NULL        | 失败                      |
| 非NULL      | OTA句柄                   |



##### IOT_OTA_Deinit

反初始化OTA模块, 释放所有资源

```c
int IOT_OTA_Deinit(void *handle)
```

| 参数       | 说明    |
| ---------- | ------- |
| handle     | OTA句柄 |
| **返回值** |         |
| 0          | 成功    |
| < 0        | 失败    |



##### IOT_OTA_ReportVersion

向云端上报当前SDK版本号

```c
int IOT_OTA_ReportVersion(void *handle, const char *version);
```

| 参数       | 说明 |
| ---------- | ---- |
|            |      |
| **返回值** |      |
|            |      |



##### IOT_OTA_ReportProgress

向云端上报升级进度

```c
int IOT_OTA_ReportProgress(void *handle, IOT_OTA_Progress_t progress, const char *msg);
```

| 参数       | 说明    |
| ---------- | ------- |
| handle     | OTA句柄 |
| version    | 版本号  |
| **返回值** |         |
| 0          | 成功    |
| < 0        | 失败    |

###### 参数附加说明

```c
typedef enum {
    IOT_OTAP_BURN_FAILED = -4,
    IOT_OTAP_CHECK_FALIED = -3,
    IOT_OTAP_FETCH_FAILED = -2,
    IOT_OTAP_GENERAL_FAILED = -1,
    IOT_OTAP_FETCH_PERCENTAGE_MIN = 0,
    IOT_OTAP_FETCH_PERCENTAGE_MAX = 100
} IOT_OTA_Progress_t;
```

- `IOT_OTAP_BURN_FAILED`: 固件烧写失败
- `IOT_OTAP_CHECK_FALIED`: 固件校验失败
- `IOT_OTAP_FETCH_FAILED`: 固件下载失败
- `IOT_OTAP_GENERAL_FAILED`: OTA其他错误
- `IOT_OTAP_FETCH_PERCENTAGE_MIN`: 固件OTA进度最小值, 值为0
- `IOT_OTAP_FETCH_PERCENTAGE_MAX`: 固件OTA进度最大值, 值为100
- 升级进度取值范围为: [0,100]区间的整数



##### IOT_OTA_IsFetching

检测当前OTA模块是否处于从云端获取数据的状态

```c
int IOT_OTA_IsFetching(void *handle);
```

| 参数       | 说明               |
| ---------- | ------------------ |
| handle     | OTA句柄            |
| **返回值** |                    |
| 0          | 未处于获取数据状态 |
| 1          | 正在从云端获取数据 |



##### IOT_OTA_IsFetchFinish

检测当前OTA模块是否获取数据完成

```c
int IOT_OTA_IsFetchFinish(void *handle);
```

| 参数       | 说明                 |
| ---------- | -------------------- |
| handle     | OTA句柄              |
| **返回值** |                      |
| 0          | 获取数据未完成       |
| 1          | 已从云端获取数据完成 |



##### IOT_OTA_FetchYield

该接口用于从网络接收报文, 需要用户周期性调用

```c
int IOT_OTA_FetchYield(void *handle, char *buf, uint32_t buf_len, uint32_t timeout_s);
```

| 参数       | 说明                         |
| ---------- | ---------------------------- |
| handle     | OTA句柄                      |
| buf        | 用于获取配置文件的临时buffer |
| buf_len    | `data_buf`的长度             |
| **返回值** |                              |
| 0          | 成功                         |
| < 0        | 失败                         |



##### IOT_OTA_Ioctl

用于设置OTA部分参数或获取当前OTA运行状态

```c
int IOT_OTA_Ioctl(void *handle, IOT_OTA_CmdType_t type, void *buf, size_t buf_len);
```

| 参数       | 说明        |
| ---------- | ----------- |
| handle     | OTA句柄     |
| type       | 命令类型    |
| buf        | 命令buffer  |
| buf_len    | `buf`的长度 |
| **返回值** |             |
| 0          | 成功        |
| < 0        | 失败        |

###### 参数附加说明

```c
typedef enum {
    IOT_OTAG_COTA_CONFIG_ID,
    IOT_OTAG_COTA_CONFIG_SIZE,
    IOT_OTAG_COTA_SIGN,
    IOT_OTAG_COTA_SIGN_METHOD,
    IOT_OTAG_COTA_URL,
    IOT_OTAG_COTA_GETTYPE,
    IOT_OTAG_OTA_TYPE,
    IOT_OTAG_FETCHED_SIZE,     /* option for get already fetched size */
    IOT_OTAG_FILE_SIZE,        /* size of file */
    IOT_OTAG_MD5SUM,           /* md5 in string format */
    IOT_OTAG_VERSION,          /* version in string format */
    IOT_OTAG_CHECK_FIRMWARE,    /* Check firmware is valid or not */
    IOT_OTAG_CHECK_CONFIG,      /* Check config file is valid or not */
    IOT_OTAG_RESET_FETCHED_SIZE  /* reset the size_fetched parameter to be 0 */
} IOT_OTA_CmdType_t;
```

- `IOT_OTAG_COTA_CONFIG_ID`: 当前可升级配置文件ID
- `IOT_OTAG_COTA_CONFIG_SIZE`: 当前可升级配置文件大小
- `IOT_OTAG_COTA_SIGN`: 当前可升级配置文件签名
- `IOT_OTAG_COTA_SIGN_METHOD`: 当前可升级配置文件计算签名的方法
- `IOT_OTAG_COTA_URL`: 当前可升级配置文件下载地址
- `IOT_OTAG_COTA_GETTYPE`: 当前可升级配置文件类型
- `IOT_OTAG_OTA_TYPE`: 当前OTA类型, COTA或FOTA
- `IOT_OTAG_FETCHED_SIZE`: 当前可升级配置文件已下载大小
- `IOT_OTAG_FILE_SIZE`: 当前可升级固件文件大小
- `IOT_OTAG_MD5SUM`: 当前已下载文件MD5值
- `IOT_OTAG_VERSION`: 当前可升级固件版本号
- `IOT_OTAG_CHECK_FIRMWARE`: 对已下载固件文件进行校验
- `IOT_OTAG_CHECK_CONFIG`: 对已下载配置文件进行校验
- `IOT_OTAG_RESET_FETCHED_SIZE`: 下载开始前, 将已下载的数据量大小size_fetched值清零



##### IOT_OTA_GetLastError

获取最近一次的错误码

```c
int IOT_OTA_GetLastError(void *handle);
```

| 参数       | 说明                 |
| ---------- | -------------------- |
| handle     | OTA句柄              |
| **返回值** |                      |
| 0          | 最近一次错误码为成功 |
| < 0        | 最近一次错误码为失败 |

