ACW(Auto Config Wi-Fi) - 自动配置Wi-Fi套件

___

## 简介

早期Wi-Fi设备基本是带有屏幕和输入设备的，配网时只需要用户直接输入路由器的SSID/password即可；随着物联网的兴起，Wi-Fi被应用于没有人机交互接口的设备，因此催生了自动配网Wi-Fi的需求。首先澄清几个概念：

- 狭义配网：Wi-Fi设备获取路由器信息(SSID,password等)并连接路由器的过程；
- 绑定：用户手机APP账号与被配网设备关联的过程；
- 广义配网：狭义配网+绑定。

目前市场上常见的现有配网方案如下：

### 1 一键配网（airkiss）

一键配网使用广播帧(配网信息编码在长度字段中，有效位11bits)/组播帧(配网信息编码在组播地址中，有效位23bits)。配网按键使能后，设备进入监听模式，分时扫描设备所支持的所有信道；捕获到符合配网规则的数据后停止信道轮询，尝试在此信道上接收完所有报文，解析出SSID和password。

优点：用户操作简单，体验好。

缺点：对手机，路由器都要严格的兼容性要求，如路由器不支持广播/组播转发；5G、2.4G Wi-Fi互通性问题。

### 2 设备热点配网

待配设备开启一个softAP，SSID及password预先约定好；APP扫描到约定SSID，并与其建立连接，将配网信息发送个待配设备，待配设备用配网信息连接路由器。

优点：成功率高，可靠。

缺点：用户操作相对较复杂（iphone用户需要收到连接热点）。

### 3 蓝牙配网

跟设备热点配网流程差不多，采用ble替换了设备端的热点。

优点：兼容性较好，成功率高，用户体验好。

缺点：成本略高，需要端侧额外增加蓝牙成本。

### 4 路由器配网

该方案类似andlink配网，该方案下，路由器在配网模式下开启一个特定的用于配网的SSID，设备发现了这个热点后连接该热点以获取配网信息。

![](doc\router_cfg.png)

<h6 style="text-align:center">图1 路由器配网方案</h6>
优点：用户体验好。

缺点：应用面窄、需满足设备跟路由器都为同一方案场景，适合生态厂商玩。

### 5 零配

零配配网方案需要主配和待配设备的应用程序都能发送和接收802.11管理帧。直接使用802.11 的管理帧(probe request/response)携带配网信息，从而实现高效配网，其原理如图2所示。

![](doc\zero_conf_mgmt.png)

<h6 style="text-align:center">图2 零配管理帧配网方案</h6>
优点：用户体验好，成功率高。

缺点：需要满足路由器下已经存在配网的同方案的设备要求，第一个设备配网方式还需要采用上面提到的方案来实现。

### 6 ACW设计方案

基于以上设计方案，One-OS提出了修改版的零配配网方案，为了提高方案的普适性，我们方案中不用管理帧交互配网信息，采用AP/STA的模式进行配网信息交互。ACW是一个设备热点配网+零配方案的综合体，其设计架构如图所示。

![](doc\acw_architecture.png)

<h6 style="text-align:center">图3 ACW设计架构</h6>
![](doc\first_dev_router[without +acw].png)

<h6 style="text-align:center">图4 主设备配网数据流</h6>
![](doc\more_dev_router[without +acw].png)

<h6 style="text-align:center">图5 零配设备配网数据流</h6>
ACW设计架构及数据流如图3-图5所示，该方案属于广义配网范畴。

## 目录结构

ACW源代码目录结构如下表所示：

| 目录             | 说明                                 |
| ---------------- | ------------------------------------ |
| include          | 头文件                               |
| source           | 自动配方案实现                       |
| source/plat_demo | 设备侧接入测试平台代码及模拟设备代码 |

从ACW组件设计架构上看，主要分为端侧、平台侧(云)和人机交互侧。source目录下代码为自动配网端侧代码；source/plat_demo目录下代码为端侧和平台侧交互代码，为了配合自动配网演示，已搭建有一套演示平台，用户可以接入演示平台体验自动配网功能，演示平台配套有配网小程序和5G消息人机交互程序，微信配网小程序二维码如图6所示，5G消息中可以实现设备控制及交互，需要在支持5G消息的手机上体验。目前演示平台支持4种设备类型分别为空调(ac)/洗衣机(wm)/床头灯(lp)/插座(pp)，设备侧需要配置8为设备ID，ID前2个字节表示设备类型，请使用acw_set_devid dev_id命令设置设备ID，测试服务器配置请使用命令设置acw_set_mqtt_info 10.15.17.145 1883 admin M!0l&Tv9ct。目前端侧代码，端侧和平台交互代码已经解耦，用户可以接入自己平台，在自动配网中注册端侧和平台交互接口。

![](doc\applet of acw.png)

<h6 style="text-align:center">图6 微信配网小程序</h6>
## 图形化配置

使用acw需要通过Menuconfig的图形化工具进行配置选择，配置的路径如下所示：

```
(Top) → Components→ Network→ Acw
                                   OneOS Configuration
[*] Enable auto config WIFI
[*]     Enable acw demo
[ ]     Enable passwd crypto

```

Enable auto config WIFI选项为开启端侧自动配网功能；Enable acw demo选项为开启端侧接入演示平台功能，如果用户接入自己开发平台，该选项不需要开启；Enable passwd crypto为配网信息加密需要，默认不加密，演示平台不支持配网信息加密，无需开启该选项，如果用户自己平台需要考虑配网信息加密，可开启该选项，平台侧需要实现对设备私钥管理和下发。

## 接口说明

| 接口                                                         |                                                              |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| typedef void (*acw_dev_connect_home_succ_sync)(char *home_id, os_bool_t clr); | 设备连接并加入家庭成功通知，为函数指针，对接用户私有平台时需要外部传入 |
| typedef void (*acw_dev_req_add_home)(char *dev_id, os_bool_t clr, os_uint8_t rand); | 设备向平台请求加入家庭，为函数指针，对接用户私有平台时需要外部传入 |
| void acw_start_proc(acw_intf_t intf, acw_dev_connect_home_succ_sync cnt_succ, acw_dev_req_add_home add_home); | 开启自动配网流程，intf参数代表接口类型，当前仅支持MCU+ESP8266的硬件方案 |
| void acw_dev_req_add_home_result_sync(char *dev_id, os_bool_t is_appoved, acw_passwd_enc_type_t enc_type, char *key); | 用户是否允许设备加入家庭中，私有平台解析用户指令结果后，同步给acw端侧程序 |
| void acw_clear_conf_notice(void);                            | 通知acw主程序，清除当前Wi-Fi配置信息                         |
| os_bool_t acw_get_clear_conf_flag(void);                     | 获取Wi-Fi配置信息清除标志                                    |

## 组件依赖

1. Molink，选择Wi-Fi模组esp8266
2. Socket
3. Easyflash
4. MQTT & MQTT Client

## 应用示例

1、包含头文件

```
#ifdef NET_USING_ACW_DEMO
#include "plat_demo.h"
#endif
```

2、启动demo

```
static void user_task_pre(void)
{
#ifdef NET_USING_ACW_DEMO
	acw_plat_demo_init(acw_intf_molink_esp8266);
    while (1)
    {
        os_task_msleep(200);
    }
#endif
}
```

3、主程序中加载demo

```
int main(void)
{
    os_task_t *task;

	user_task_pre();

    task = os_task_create("user", user_task, NULL, 512, 3);
    OS_ASSERT(task);
    os_task_startup(task);

    return 0;
}
```