## paho-mqtt更改说明

## 组件说明：

源码包来源是Eclipse Paho MQTT Embedded C源码包，版本pahomqtt-v1.1.0

## 2021-01-20 更改说明:

- 适配OneOS-V2.0版本。

## 2020-12-30 更改说明:

- 重构mqtt tls network相关接口的实现。
- 新增两个mqtt network接口：MQTTNetworkConnect和MQTTNetworkDisconnect。

## 2020-12-23 更改说明:

* 修改Kconfig和SConscript，将paho-mqtt的协议层与客户端层分开配置。

## 2020-06-08 更改说明：

- paho-mqtt 源码包适配 OneOS 操作系统
- 增加TLS加密传输方式（只支持单向认证）