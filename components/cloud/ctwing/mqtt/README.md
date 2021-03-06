# CTWing-MQTT用户指南

------

## 简介

CTWing-MQTT组件是OneOS对天翼物联网平台MQTT接入SDK的适配与开发，支持明文传输和TLS加密传输、支持设备消息透传模式和使能服务模式。

## 目录结构

CTWing-MQTT 源代码目录结构如下表所示：

| 目录                         | 说明                         |
| ---------------------------- | ---------------------------- |
| src/ctiot_client             | ctiot mqtt 设备相关接口实现  |
| src/MQTTClient               | 鉴权相关接口实现             |
| src/platform                 | 与操作系统相关的接口适配     |
| mqtt_sdk_sample/ctiot_client | 演示样例及演示设备的相关信息 |
| certs/src                    | 平台根证书                   |

## 架构

**流程**

![ctwing_mqtt](doc\images\ctwing_mqtt.png)

- **设备初始化**：包含获取设备信息、获取指令下发的回调函数指针。
- **设备登录平台**：包含网络初始化、MQTT客户端初始化、网络连接（tcp/tls）、MQTT客户端连接、订阅及回调函数注册。网络连接失败、MQTT客户端连接失败、订阅失败的话会循环进行这三个步骤。
- **消息接收、命令回调及心跳维护**：自动维护心跳包，接收平台的消息，如指令下发，并执行相应的回调函数。
- **数据上报、事件上报**：发布用户的消息，如数据上报、心跳上报。

## CTWing平台相关

### 属性与服务介绍

设备的服务是用来描述一款设备是什么、能做什么以及如何控制该设备的文件。一个服务文件的内容主要包括设备的服务信息，这一部分定义了设备的业务数据，包括设备上报的上行数据和厂商服务器下发给设备的下行数据。每款设备都需要一个数据集文件。

对于MQTT协议，**非透传**产品设备均需在“服务定义”标签页内添加属性定义及服务定义，即**使能服务模式**。

#### 属性列表

在属性列表中，点击“**新增属性**”添加设备属性。属性值主要用于描述设备属性，并在数据上报时更新内部设备影子，为后续规则引擎模块提供参数。

属性标识符唯一代表某一属性，产品下具有唯一性，其格式要求须符合规范，且需认真填写，一经生成不允许修改。属性定义主要包括7种数据类型。

#### 服务列表

在服务列表中，点击“**新增服务**”添加设备服务，主要用于定义设备某一具体的服务。

服务种类主要分为数据上报、事件上报、指令下发及指令下发响应等四种，用户根据实际业务场景进行选择。服务标识符唯一代表某一服务，产品内不允许重复，且需按照要求规范填写，一经生成不允许修改。

定义服务时，参数的顺序按照实际报文的payload字段顺序进行添加。对于数据上报类型，参数仅允许从属性列表中选择添加。其他类型服务可由用户选择新建参数或从属性中选择。

每添加完一项服务，均会根据定义的服务标识符对应生成该服务类型的topic主题，并在"**topic列表**"标签页内对应更新。topic在某一产品下具有唯一性，在做具体的业务数据操作时，必须使用定义的topic。

**说明：**

**服务可以根据实际业务场景的变化进行增删改操作，且同一产品下的服务共用。**

**若某一属性已被服务征用，则不允许对其进行任何操作。**

**一个服务唯一对应一条报文，一个服务可包含多个属性或参数。**

### 使能服务模式

使能服务模式，即对于非透传产品下创建的设备，必须在门户上进行属性定义及服务定义。

门户中定义的属性列表，即为之后可进行规则引擎的数据列表，原则上需在数据上报报文中涵盖响应的属性才能使用后续规则引擎的能力。

门户中定义的服务列表，需按照实际业务数据选择服务类型，每一条服务唯一对应一个topic主题，即服务标识符就是topic主题名称。设备的业务数据需根据定义，进行topic的填写。同时，payload部分的json结构体，需按照门户定义的key/value值填写，否则可能会出现数据不合法而丢弃的现象。

**->数据上报**：用户可在门户自定义“数据上报”类型的服务，生成相应topic，publish报文的topic字段填写定义的topic，且支持Qos0/1/2三种质量等级数据上报

**->事件上报**：用户可在门户自定义“事件上报”类型的服务，生成相应topic，publish报文的topic字段填写定义的topic，且支持Qos0/1/2三种质量等级上报

**->指令下发**：用户可在门户自定义“指令下发”类型的服务，生成相应topic，平台会自动为每个MQTT设备订阅这些topic主题，平台下发的所有指令质量等级为Qos1。

设备收到的指令mqtt协议报文payload格式：

{"taskId":64004,"payload":{"dn":"AB"}}

其中，"taskId"是指令序号，具有全局唯一性。"payload"字段内为实际下发的指令内容。

**->指令下发响应**：用户可在门户自定义“指令下发响应”类型的服务，生成相应topic，publish报文的topic字段填写定义的topic，且支持Qos0/1/2三种质量等级上报。

设备回复的mqtt协议报文payload格式：

{"taskId":64004,"resultPayload":{"rsp":"AB"}}

taskId需与平台指令下发中的一致，可在指令报文中直接获取。resultPayload字段内为具体回复的业务层数据，业务数据需与平台中定义的服务要求一致。

### 透传模式

创建透传产品，无需进行属性定义和服务定义。

**->数据上报**：用户可在publish报文的topic字段填写任意主题，且支持Qos0/1/2三种质量等级数据上报

**->指令下发**：平台会自动为每个MQTT设备订阅device_control主题，平台下发的所有指令均为此主题，且质量等级为Qos1。设备端需支持对device_control主题报文的处理。平台不支持对透传设备指令响应的处理，原则上没有下发指令的上行响应报文，若有需要，用户需自行在应用侧做区分处理。

### 支持数据类型

添加服务时，可以根据实际业务场景选择数据类型，目前AEP支持解析的数据类型包括以下几种：

1、整型Int32  4Byte  -2147483648 - 2147483647

2、单精度浮点型 Float  4Byte

3、双精度浮点型 Double 8Byte

4、枚举值 enum 1byte 0~255

5、布尔值 bool

6、字符串 string

7、时间 date

### Topic列表

#### 使能服务模式Topic列表 ：

**注：与具体定义的产品属性相关**

|      | **系统topic**      |           **用途**           | **QoS** | **可订阅** | 可发布 |
| ---- | ------------------ | :--------------------------: | ------- | :--------- | ------ |
| 1    | info_report        |   设备信息上报（数据上报）   | 0/1/2   | —          | √      |
| 2    | data_report        |    温湿度上报（数据上报）    | 0/1/2   | —          | √      |
| 3    | ir_sensor_report   |  红外传感器上报（事件上报）  | 0/1/2   | —          | √      |
| 4    | motor_control_cmd  |     电机控制（指令下发）     | 1       | √          | —      |
| 5    | motor_control_resp | 电机控制响应（指令下发响应） | 0/1/2   | —          | √      |

#### 透传模式Topic列表 ：

|      | **系统topic**  | **用途** | **QoS** | 可订阅 | **可发布** |
| ---- | -------------- | :------: | ------- | ------ | ---------- |
| 1    | 任意主题       | 数据上报 | 0/1/2   | —      | √          |
| 2    | device_control | 指令下发 | 1       | √      | —          |

### 加密与非加密

| **连接协议** | **地址**       | **端口** | **说明**                |
| ------------ | -------------- | -------- | ----------------------- |
| MQTT         | mqtt.ctwing.cn | 1883     | 非加密端口              |
| MQTT         | mqtt.ctwing.cn | 8883     | 加密端口（TLS单向认证） |

## 使用说明

### 图形化配置

使用CTWing-MQTT组件需要通过Menuconfig的图形化工具进行配置选择，配置的路径如下所示：

```
(Top) → Components→ Cloud→ CTWing→ MQTT
[*] Enable ctwing mqtt
[*]     Enable ctwing mqtt TLS encrypt
[*]     Enable device echo server command
[ ]     Enable device transparent transport
        The ctiot mqtt global output log level. (Debug)  --->
[*]     Enable ctiot mqtt demo
() Config demo device number
() Config demo device id
() Config demo device token
```

进行CTWing-MQTT组件选项配置需要先在Menuconfig中选中Enable ctwing mqtt，然后再进行其他的配置选择。

- Enable ctwing mqtt TLS encrypt：使能TLS加密传输功能。
- Enable device echo server command：使能设备响应平台命令下发功能。
- Enable device transparent transport：使能设备消息透传功能。
- The ctiot mqtt global output log level：选择log打印等级。
- Enable ctiot mqtt demo：使能演示示例。
- Config demo device number：配置设备编号
- Config demo device id：配置设备ID
- Config demo device token：配置连接认证信息-特征串

### API使用说明手册

[CTWing MQTT API使用说明手册](doc/ctwing_mqtt_api.md)

## 注意事项

#### 1. 关闭CTWing MQTT 组件

开启CTWing MQTT 组件后会自动勾选Paho MQTT组件，当用户反向取消该组件时记得手动关闭Paho MQTT组件。

#### 2. 关闭CTWing MQTT 的加密传输

开启CTWing MQTT 的加密传输功能后会自动勾选mbedtls加密组件，当用户反向取消CTWing MQTT 的加密传输功能时记得手动关闭mbedtls组件。