# Baidu Cloud Kit-V1.0.0

## 简介

百度云组件是OneOS操作系统提供的百度云接入组件，是对百度云发布的适用于嵌入式设备的SDK的移植与开发，通过主流的物联网协议（如 MQTT）通讯，可以在智能设备与云端之间建立安全的双向连接，快速实现物联网项目。

V1.0.0功能简介：

- 百度端云互通组件SDK移植
- 支持MQTT(S)协议接入，支持设备注册，消息订阅，消息发布
- 支持单向认证(PSK)、双向认证(CA)两种认证方式接入百度云

## 代码结构

| 目录           | 说明                                                 |
| -------------- | ---------------------------------------------------- |
| certs          | 证书相关文件，包含服务器根证书及设备证书、私钥。     |
| c-utility      | 提供基础的功能模块（比如string，列表操作，IO等等）。 |
| doc            | 说明文档                                             |
| iotcore_client | 设备端示例代码                                       |
| parson         | 应用层解析，主要是解析json文件，实现消息互通         |
| serializer     | 序列化、反序列化函数库                               |
| umqtt          | mqtt接入实现，包含设备连接、订阅、发布等功能         |

## MQTT(S)协议连接

MQTT（Message Queuing Telemetry Transport，消息队列遥测传输协议），是一种基于发布/订阅（publish/subscribe）模式的"轻量级"通讯协议，该协议构建于TCP/IP协议上，由IBM在1999年发布。MQTT最大优点在于，可以以极少的代码和有限的带宽，为连接远程设备提供实时可靠的消息服务。作为一种低开销、低带宽占用的即时通讯协议，使其在物联网、小型设备、移动应用等方面有较广泛的应用。

MQTT是一个基于客户端-服务器的消息发布/订阅传输协议。MQTT协议是轻量、简单、开放和易于实现的，这些特点使它适用范围非常广泛。在很多情况下，包括受限的环境中，如：机器与机器（M2M）通信和物联网（IoT）。其在，通过卫星链路通信传感器、偶尔拨号的医疗设备、智能家居、及一些小型化设备中已广泛使用。

## 系统限制

#### 连接及消息收发相关

| 限制描述                     | 限制值                        | 说明                           |
| ---------------------------- | ----------------------------- | ------------------------------ |
| 单项目最大同时在线长连接数   | 1万                           | -                              |
| 单实例每秒新建MQTT连接请求数 | 200                           | -                              |
| 单MQTT连接的最大订阅数       | 100                           | 超过订阅数的请求将会被直接拒绝 |
| 单连接客户端每秒订阅请求数   | 10                            | 每秒最多进行10次订阅主题的操作 |
| 单连接上报消息频率上限       | QoS0：30条/秒，QoS1：10条/秒  | -                              |
| 离线消息最长缓存时间         | 1周                           | -                              |
| 单个连接每秒的吞吐带宽       | 512 KB                        | -                              |
| 每个订阅请求的最大订阅数     | 8                             | -                              |
| MQTT单个发布消息最大长度     | 32 KB                         | -                              |
| MQTT连接心跳时间             | 30至1200秒，建议取值300秒以上 | -                              |
| ClientID 长度                | 128字节                       | -                              |

#### 主题（Topic）相关

| 限制描述                   | 限制值 | 说明                                                  |
| -------------------------- | ------ | ----------------------------------------------------- |
| Topic总长度                | 255    | -                                                     |
| Topic每层长度              | 40     | -                                                     |
| 每个Topic可被订阅上限      | 200    | 即一个主题匹配的订阅连接数最多为200，超出部分将不推送 |
| 订阅和取消订阅操作生效时间 | 5s     | 即 Client 向服务端发送 sub 请求最长 5s 后订阅生效     |

#### 设备限制

| 限制描述       | 限制值 | 说明 |
| -------------- | ------ | ---- |
| 单实例设备数量 | 10w    | -    |

## 配置选项

使用baidu cloud kit需要通过Menuconfig的图形化工具进行配置选择，配置的路径如下所示：

```text
(Top) → Components→ Cloud→ Baidu
                                              OneOS Configuration
[*] Baidu iotcore  --->
    *** Enable Baidu IoT Demo ***
[*] Enable/Disable TLS
        Secret Key Type (PSK)  --->
(xxx.iot.gz.baidubce.com) Config Server Address (NEW)
(iot core id) Config IoTCore ID (NEW)
(device key) Config Device Key (NEW)
(device secret) Config Device Secret (NEW)
```

##### 配置入口

​        配置路径：Components→ Cloud→ Baidu

##### 参数说明

​		Baidu iotcore:使能百度云连接组件

​		Select method ：选择模式：

​									1. Demo，提供百度云连接Demo，接入百度云并进行数据交换

​		Enable/Disable TLS：使能/去使能TLS，去使能TLS后，所有报文将以明文形式发送，建议使能TLS；

​		Secret Key Type：使能TLS后，用户可选择单向认证模式，或双向认证模式

​									1. PSK，单向认证模式，需要用户从云平台下载设备密钥

​									2. CA，双向认证模式，需要用户从云平台下载设备证书及设备私钥

​		Config Server Address：配置百度云服务器接入地址；

​		Config IoTCore ID：产品ID，百度云设备注册后，云平台提供；

​		Config Device Key：设备ID，百度云设备注册时用户输入；

​		Config Device Secret：设备密钥，百度云设备注册后，云平台提供（仅PSK模式下需要）；

​		

## 云平台操作步骤

#### 使用前准备

- [创建百度智能云账号](https://login.bce.baidu.com/)：并根据提示完成实名认证。
- [开通IoTCore](https://console.bce.baidu.com/iot2/core/landing)：开通服务并同意按需计费，可进入“实例列表”。

#### 操作流程

![](.\docs\figures\image1.jpg)

#### 创建IoT Core实例

选择[物联网核心套件IoT Core](https://console.bce.baidu.com/iot2/core/core/list)，点击实例列表，并选择“创建IoT Core”

![](.\docs\figures\image5.jpg)

#### 创建设备

前置条件：创建模板，创建设备前需要先添加模板，为本实例创建default topic，请记录该topic，设备连接云平台后，可订阅/发布该主题。

![](.\docs\figures\image2.jpg)

模板创建完成后，可以在设备列表中添加新设备

![](.\docs\figures\image3.jpg)

- 名称，设备的Device Key，工程配置时需要填入配置项。
- 认证方式分CA证书PSK密码认证：
  - 选择证书认证时，设备创建完成后，可以从百度智能云平台下载设备证书及私钥，并将该文档存在到 components\cloud\baidu\certs\ClientCert&Key\cert-and-keys.txt文件中。
  - 选择密钥认证时，设备创建完成后，可以从百度智能云平台下载Device Secret，工程配置时，需要填入配置项。
- 模板，选择用户自定义的模板，用户可以在业务代码中订阅或发布该主题，该主题名可以在模板的详细信息中获取。

#### 获取连接信息

在实例列表中，“接入点”选项可以获取云平台服务器地址，在工程配置时，需填入Server Address选项

![](.\docs\figures\image4.jpg)

#### 连接及消息收发

开发者可以根据业务需要，选择mqtt、mqtts(证书)、mqtts(密钥)三种方式接入云平台，并进行业务收发。



## 注意事项

1. 百度云不支持消息在线查看，用户可以通过订阅设备发布的消息来确认设备与云平台之间的连接状态；
3. 百度云服务器连接超时时间为10秒，采用TLS方式接入时，使用外部RAM时容易超时，建议采用内部RAM较大的MCU（如：正点原子-阿波罗开发板）

