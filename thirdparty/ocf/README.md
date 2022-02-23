# OCF IOTivity使用说明



## 1.网络

OCF IOTivity运行需要网络，目前支持ETH, MOLINK两种联网方式。

ETH menuconfig配置：

(Top) → Drivers→ HAL→ Enable Ethernet

(Top) → Components→ Network→ TCP/IP→ LwIP

MOLINK menuconfig配置：

(Top) → Components→ Network→ Molink→ Enable IoT modules support → Module→ WiFi Modules Support

## 2.实例

目前提供有空调，空气净化器，摄像头，灯，门锁，热水器，洗衣机7种实例，在menuconfig

(Top) → Thirdparty→ Iotivity→ Iotivity: OCF Iotivity library → ocf device type下开启相应实例。

## 3.PKI

若要使用PKI，在menuconfig

(Top) → Thirdparty→ Iotivity下打开OCF  PKI

## 4.数据存储

OCF IOTivity需要flash存储数据，ONEOS提供有easyflash模块，直接打开该模块并配置分区（flash尽量使用片内flash）即可使用，

在menuconfig

(Top) → Thirdparty→ Easyflash→ EasyFlash: Lightweight embedded flash memory library. → on chip flash下配置easyflash

在/board/ports/fal_cfg.c中配置分区，分区名为easyflash

```
static const fal_part_info_t fal_part_info[] = 

{

  /*    part,     flash,    addr,    size,            lock */

  {    "app", "onchip_flash", 0x00000000, 0x000c0000, FAL_PART_INFO_FLAGS_UNLOCKED},

  { "easyflash", "onchip_flash", 0x000c0000, 0x00040000, FAL_PART_INFO_FLAGS_UNLOCKED}

};
```

## 5.RAM

OCF IOTivity对于运算效率要求比较高，需要使用SRAM，若系统的heap默认配置为SDRAM，可在menuconfig

(Top) → Thirdparty→ Iotivity下打开OCF use custom ram

## 6.启动

OCF已在ocf_cmd.c实现了一个任务，在系统启动后会自动运行，任务名为ocf_server，也可自行在main中启动。

启动后可以通过OCF客户端发现设备，onboard设备后，可对设备进行操作。



# 示例：STM32H743开发板OCF工程配置步骤

基于ONEOS 2.0的stm32h743-atk-apollo标准模板进行OCF工程menuconfig配置

1.(Top) → Drivers→ SPI下关闭全部SPI选项

2.(Top) → Drivers→ Sensors下关闭全部sensor选项

3.(Top) → Drivers→ Serials下将串口的RX buffer size设置为8192，其他不变

4.(Top) → Components→ Network→ Socket下打开Enable BSD socket API选项

5.(Top) → Components→ Network→ Molink下打开Enable IoT modules support

6.(Top) → Components→ Network→ Molink→ Enable IoT modules support → Module→ WiFi Modules Support→ ESP8266 → ESP8266 Config下进行ESP8266配置，除了最后一个Enable ESP8266 Module Hardware Control Operates之外，其他的选项全部打开，串口设置为uart3，ssid和passwd设置为自己用的wifi，其他选项用默认值

7.(Top) → Thirdparty→ Iotivity下打开Iotivity: OCF Iotivity library选项，ocf device type选择air conditioner，后面的依次选中

Iotivity: OCF ipv4 support

Iotivity: OCF debug

Iotivity: OCF server

Iotivity: OCF security

Iotivity: OCF dynamic allocation

Iotivity: OCF use oneos 2.0

8.(Top) → Components→ Dlog→ Enable dlog → The log global output level.下设置dlog日志级别为Warning

9.保存并退出menuconfig，执行scons --ide=mdk5生成keil工程

10.打开keil工程，点击option按钮，进入工程option配置页，打开C/C++选项卡，勾选GNU extensions，将编译优化等级改为-O3，如果使用的ST-LINK，还需打开Debug选项卡，选择ST-Link Debugger，然后进入settings，打开Flash Download选项卡，点击下方的add按钮，添加flash，flash选择STM32H7x_2048即可，保存退出option

11.打开stm32h743-atk-apollo模板下board/ports/fal_cfg.c文件，修改分区配置为

```
static const fal_part_info_t fal_part_info[] = 

{

  /*    part,     flash,    addr,    size,            lock */

  {    "app", "onchip_flash", 0x00000000, 0x001C0000, FAL_PART_INFO_FLAGS_UNLOCKED},

  { "ocf_data", "onchip_flash", 0x001C0000, 0x00040000, FAL_PART_INFO_FLAGS_UNLOCKED}

};
```

12.编译工程，下载固件

13.OCF服务会自启动，任务名为ocf_server

【备注】：

1.每次执行scons --ide=mdk5生成工程后，需要重新执行第9步，也可以将第9步改动到keil工程template里面，这样就无需每次都去修改

<u>2.esp8266模组需要使用特定的固件版本：AT version:2.1.0.0-dev，SDK version:v3.3-rc1-64-gfaae0a99，Bin version:2.0.0(WROOM-02)</u>

<u>3.执行pki用例前需要设置rtc时间，具体值为：当前时间 - 8小时</u>



# 示例：STM32F767开发板OCF工程配置步骤

基于ONEOS 2.0的stm32f767-atk-apollo标准模板进行OCF工程menuconfig配置

1.(Top) → Drivers→ Serials下将串口的RX buffer size设置为8192，其他不变

2.(Top) → Thirdparty→ Iotivity下打开Iotivity: OCF Iotivity library选项，ocf device type选择air conditioner，后面的依次选中

Iotivity: OCF ipv4 support

Iotivity: OCF debug

Iotivity: OCF server

Iotivity: OCF security

Iotivity: OCF dynamic allocation

Iotivity: OCF use oneos 2.0

3.保存并退出menuconfig，执行scons --ide=mdk5生成keil工程

4.打开keil工程，点击option按钮，进入工程option配置页，打开C/C++选项卡，勾选GNU extensions，将编译优化等级改为-O3，如果使用的ST-LINK，还需打开Debug选项卡，选择ST-Link Debugger，保存退出option

5.在\components\net\ip\lwip-2.1.2\src\port\netif\ethernetif.c中增加如下函数

```
void lwip_get_intf_ipaddr(char *intf_name, ip_addr_t *addr)
{
    struct netif *netif;
    int do_res;

    netif = OS_NULL;
    
    addr->addr = 0;
    os_schedule_lock();
    
    netif = netif_list;
    while (OS_NULL != netif)
    {
        do_res = memcmp(netif->name, intf_name, sizeof(netif->name));
        if (!do_res)
        {
            addr->addr = netif->ip_addr.addr;
            break;
        }
        netif = netif->next;
    }
    
    os_schedule_unlock();
    
    return;

}
```

6.在\drivers\hal\st\STM32F7xx_HAL\STM32F7xx_HAL_Driver\Src\stm32f7xx_hal_eth.c:1878行修改参数

```
//macinit.MulticastFramesFilter = ETH_MULTICASTFRAMESFILTER_PERFECT;
macinit.MulticastFramesFilter = ETH_MULTICASTFRAMESFILTER_NONE;
```

7.打开stm32f767-atk-apollo模板下board/ports/fal_cfg.c文件，修改分区配置为

	static const fal_part_info_t fal_part_info[] = 
	{
	    /*       part,            flash,       addr,       size,                       lock */
	    {       "app", "onchip_flash"  , 0x00000000, 0x000C0000, FAL_PART_INFO_FLAGS_UNLOCKED},
	    {  "ocf_data", "onchip_flash"  , 0x000C0000, 0x00040000, FAL_PART_INFO_FLAGS_UNLOCKED}
	};

8.编译工程，下载固件

9.OCF服务会自启动，任务名为ocf_server



# 示例：STM32H743演示用OCF demo使用说明

demo位于\demos\components\ocf\目录下，使用前将demo目录stm32h743-atk-apollo-ocf复制到templates目录下。

1.demo使用lvgl做gui开发，需要支持浮点数显示，打开lv_conf.h，将541行修改为

```
# define LV_SPRINTF_DISABLE_FLOAT 0
```

2.由于lvgl和lcd驱动占用的ram过多，h743的sram不足以支持demo运行，需要将lcd驱动使用的大块内存分配到sdram，打开drv_lcd.c，将93行修改为

```
lcd->graphic.info.framebuffer      = (void *)sdram_aligned_alloc(0x200, LCD_WIDTH * LCD_HEIGHT * (LCD_BITS_PER_PIXEL / 8));
```

并在static os_err_t stm32_lcd_init(struct stm32_lcd *lcd)函数前添加头文件引用

```
#include <sdram_port.h>
```

3.(Top) → Components→ Network→ Molink→ Enable IoT modules support → Module→ WiFi Modules Support→ ESP8266 → ESP8266 Config 下配置ESP 8266需要连接的wifi信息

4.(Top) → Thirdparty→ Iotivity下选择测试的设备类型，demo目前供支持6种设备，分别是：空调，空净，灯，门锁，热水器，摄像头

5.执行scons --ide=mdk5生成keil工程，然后编译，下载即可
