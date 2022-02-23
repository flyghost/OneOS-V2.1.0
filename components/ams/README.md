# ams组件

## 简介

ams组件：ams全称App Management System，是用于高级语言应用管理的组件，配合高级语言组件和APP管理分发平台一起使用。

## 代码结构

ams组件源代码相对路径为.\components\ams\，结构如下表所示：

| 文件名       | 功能     |
| ------------ | -------- |
| ams_main.c   | 程序主线程入口，提供ams中的命令处理逻辑。|
| ops          | 操作接口的代码目录，ams的驱动层， 包含协议编/解码、文件操作、网络操作的代码。|
| local        | 存放本机接口的代码目录，ams的内核层对外接口层，即在供其他模块调用的代码，如shell等交互界面。 |
| core         | 核心代码目录，ams的内核层，其内容稳定，不常变动，包含操作系统接口封装、MD5校验代码、ams任务操作代码，。|

## 功能介绍

ams组件主要提供下面功能：

1. ams与APP管理分发平台建立连接，提供平台接入功能、设备监测功能。
2. ams通过解析平台下发的各种命令，在设备上实现高级语言应用管理功能。
3. 当无云平台链接时，ams提供设备上的高级语言应用管理功能。

## 使用说明

### 图形化配置

使用ams组件可以通过Menuconfig的图形化工具进行配置选择，配置的路径如下所示：

```C
(Top) → Components→ AMS
                                        OneOS Configuration
[*] AMS: Application Managment System of High level language.
[ ]     Output the debug mode. (NEW)
        App type: The High level language used to write Application. (MicroPython)  --->
[ ]     Using APP platform management. (NEW)
(6144)  The stack szie of AMS main thread. (NEW)
(21)    The priority of AMS main thread. (NEW)
(8192)  The stack szie of AMS runner thread. (NEW)
(25)    The priority of AMS runner thread. (NEW)
```

以上配置完成后退出并保存配置，重新生成工程后，编译运行OneOS。
