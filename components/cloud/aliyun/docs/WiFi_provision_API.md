# WiFi provision API



##### awss_start

启动配网服务

```c
int awss_start(void);
```

| 参数       | 说明     |
| ---------- | -------- |
| void       |          |
| **返回值** |          |
| 0          | 成功     |
| -1         | 启动失败 |

# 

##### awss_stop

停止配网服务

```c
int awss_stop();
```

| 参数       | 说明 |
| ---------- | ---- |
| void       |      |
| **返回值** |      |
| 0          | 成功 |
| -1         | 失败 |

# 

##### awss_config_press

使能配网，开始解awss报文

```c
int awss_config_press();
```

| 参数       | 说明 |
| ---------- | ---- |
| void       |      |
| **返回值** |      |
| 0          | 成功 |
| -1         | 失败 |

# 

##### awss_dev_ap_start

启动设备热点配网，与awss_start互斥

```c
int awss_dev_ap_start();
```

| 参数       | 说明 |
| ---------- | ---- |
| void       |      |
| **返回值** |      |
| 0          | 成功 |
| -1         | 失败 |

# 

##### awss_dev_ap_stop

停止设备热点配网

```c
int awss_dev_ap_stop();
```

| 参数       | 说明 |
| ---------- | ---- |
| void       |      |
| **返回值** |      |
| 0          | 成功 |
| -1         | 失败 |

# 

##### awss_report_cloud

启动绑定服务，上报绑定信息

```c
int awss_report_cloud();
```

| 参数       | 说明 |
| ---------- | ---- |
| void       |      |
| **返回值** |      |
| 0          | 成功 |
| -1         | 失败 |

# 

##### awss_report_reset

向云端上报解除绑定消息

```c
int awss_report_reset();
```

| 参数       | 说明 |
| ---------- | ---- |
| void       |      |
| **返回值** |      |
| 0          | 成功 |
| -1         | 失败 |

# 

##### iotx_event_regist_cb

注册包括配网绑定在内的linkkit事件通知回调函数

```c
int iotx_event_regist_cb(void (*monitor_cb)(int event));
```

###### 参数附加说明

```c
void (*monitor_cb)(int event)
```

用户需要注册事件回调函数原型。

event为配网绑定相关事件，具体如下：

```c
enum iotx_event_t {
    IOTX_AWSS_START = 0x1000,       /* AWSS start without enbale, just supports device discover */
    IOTX_AWSS_ENABLE,               /* AWSS enable */
    IOTX_AWSS_LOCK_CHAN,            /* AWSS lock channel(Got AWSS sync packet) */
    IOTX_AWSS_CS_ERR,               /* AWSS AWSS checksum is error */
    IOTX_AWSS_PASSWD_ERR,           /* AWSS decrypt passwd error */
    IOTX_AWSS_GOT_SSID_PASSWD,      /* AWSS parse ssid and passwd successfully */
    IOTX_AWSS_CONNECT_ADHA,         /* AWSS try to connnect adha (device discover, router solution) */
    IOTX_AWSS_CONNECT_ADHA_FAIL,    /* AWSS fails to connect adha */
    IOTX_AWSS_CONNECT_AHA,          /* AWSS try to connect aha (AP solution) */
    IOTX_AWSS_CONNECT_AHA_FAIL,     /* AWSS fails to connect aha */
    IOTX_AWSS_SETUP_NOTIFY,         /* AWSS sends out device setup information (AP and router solution) */
    IOTX_AWSS_CONNECT_ROUTER,       /* AWSS try to connect destination router */
    IOTX_AWSS_CONNECT_ROUTER_FAIL,  /* AWSS fails to connect destination router. */
    IOTX_AWSS_GOT_IP,               /* AWSS connects destination successfully and got ip address */
    IOTX_AWSS_SUC_NOTIFY,           /* AWSS sends out success notify (AWSS sucess) */
    IOTX_AWSS_BIND_NOTIFY,          /* AWSS sends out bind notify information to support bind between user and device */
    IOTX_AWSS_ENABLE_TIMEOUT,       /* AWSS enable timeout(user needs to call awss_config_press again to enable awss) */
    IOTX_CONN_CLOUD = 0x2000,       /* Device try to connect cloud */
    IOTX_CONN_CLOUD_FAIL,           /* Device fails to connect cloud, refer to net_sockets.h for error code */
    IOTX_CONN_CLOUD_SUC,            /* Device connects cloud successfully */
    IOTX_RESET = 0x3000,            /* Linkkit reset success (just got reset response from cloud without any other operation) */
};
```

