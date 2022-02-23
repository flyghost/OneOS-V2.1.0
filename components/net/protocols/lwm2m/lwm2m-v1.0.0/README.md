# LWM2M用户指导书

## 1.功能说明

LWM2M是一种轻量级物联网设备协议，协议主要包含两个主体，一个是LWM2M Server和LWM2M Client，此外，根据需要还可以加入LWM2M引导服务器(Bootstrap Server)或智能卡(SmartCard),对客户端完成引导。当前协议支持LWM2M Client主体，可以通过LWM2M Client连接支持LWM2M Server协议的服务器。通过定时上报客户端信息或者服务器下发对客户端控制完成对设备的管理。

## 2.使用说明

LWM2M客户端支持加密和非加密方式连接服务器，可以根据LWM2M服务器支持的类型进行选择。

### 2.1menuconfig配置说明

```c
(Top) → Components→ Network→ Protocols→ LWM2M                                              [*] Enable LWM2M
[ ]     Enable LWM2M DTLS encrypt (NEW)
[*]     Enable LWM2M sample
(192.168.1.102) LWM2M Server Addr String (NEW)
(5684)      LWM2M Server Port String (NEW)
[*]     Enable LWM2M client mode
        Select log level (Debug Log Level)  --->
```

```c
(Top) → Components→ Network→ Protocols→ LWM2M→ Enable LWM2M → Select log level
( ) Error Log Level
( ) Warn Log Level
( ) Info Log Level
(X) Debug Log Level
( ) Verbose Log Level
( ) None Log Level
```

Enable LWM2M: 使能LWM2M组件;

Enable LWM2M DTLS encrypt：使能LWM2M示例，可以运行shell命令连接LWM2M服务器;

LWM2M Server Addr String：修改设置LWM2M服务器IP地址；

LWM2M Server Port String：修改设置LWM2M服务器端口地址；

Enable LWM2M client mode：使能LWM2M 客户端模式；

Select log level (Debug Log Level)：调试打印log级别，默认为debug级别；

### 2.2shell命令运行示例

LWM2M示例shell命令和功能有四个分别是：

lwm2m_data_sample：用于测试数据加密和解析接口是否正确；

lwm2m_proc_sample：测试主程序运行流程接口；

lwm2m_connect_server_sample：测试LWM2M客户端连接LWM2M服务器，用于定时更新上报数据与服务器，服务器可以通过下发命令到客户端；

lwm2m_disconnect_server_sample：测试LWM2M客户端注销LWM2M服务器连接；

