# OneNET EDP用户指南

------

## 简介

EDP (Enhanced Device Protocol增强设备协议）是OneNET平台根据物联网特点专门定制的完全公开的基于TCP的协议，可以广泛应用到家居、交通、物流、能源以及其他行业应用中。

具有以下功能特点：

1. 长连接协议；
2. 数据加密传输；
3. 终端数据点上报，支持的数据点类型为：浮点数(float)、整型（int）、字符串、JSON对象/二进制数据；
4. 平台消息下发（支持离线消息）；
5. 端到端数据转发。

## 目录结构

OneNET-EDP源代码目录结构如下表所示：

| 目录       | 说明           |
| ---------- | -------------- |
| edp_kit    | 协议层实现     |
| edp_enc    | 数据包加密实现 |
| edp_sample | 示例           |

## 使用说明

### 图形化配置

使用OneNET-EDP需要通过Menuconfig的图形化工具进行配置选择，配置的路径如下所示：

```
(Top) → Components → Cloud → OneNET → EDP
[*] Enable OneNET EDP protocal
[ ]     Enable EDP crypt
[ ]     Enable EDP sample
```

进行OneNET-EDP选项配置需要先在Menuconfig中选中Enable OneNET EDP protocal，然后再进行其他的配置选择。

- Enable EDP crypt：使能EDP协议加密传输。
- Enable EDP sample：使能EDP示例。

### API使用说明手册

[OneNET EDP API使用说明手册](doc/onenet_edp_api.md)

## 编译选项

## 注意事项

#### 1.加密选项

开启EDP的加密功能后，会自动勾选mbedtls加密组件，当反向取消EDP的加密功能时，需要手动关闭mbedtls加密组件。



