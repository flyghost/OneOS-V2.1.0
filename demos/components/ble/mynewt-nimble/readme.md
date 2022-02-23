## Nimble demo 使用方法

1. 以"demos\components\ble\mynewt-nimble\advertiser"为例。
2. 使用Cube工具中调用project命令生成相应工程，删除生成工程中的"application"文件夹。
3. 将"advertiser"中的"application"文件夹拷贝到生成的工程下。
4. 使用"advertiser"中的".config"文件的内容替换掉生成工程中".config"文件中BLE段的配置。
5. 在Cube工具中进入menuconfig刷新配置。可以更改系统任意简单配置后保存退出menuconfig，再改回原配置以刷新添加的BLE部分。

## Nimble demo 使用注意事项

1. 使用Nrf52系列平台并且使能controller时，Nimble协议栈会使用RTC0和Timer0，所以在menuconfig里不能使能Drivers->HAL下面的timer0和rtc。
2. 如果选择keil编译，keil arm compiler版本需支持version 6，并在'Options->Target'下选择'Use default compiler version 6'，并取消对'Use MicroLIB'的勾选。
3. blecsc_host和blecsc_controller分别运行在stm32l475-atk-pandora和nrf52840-DK开发板上，通过串口进行通信，stm32l475-atk-pandora上使用uart1的PA9/PA10引脚，nrf52840-DK使用uart0的P0.06/P0.08引脚。
