nrf52832-pca10040 烧录说明



1. 该项目因为使能OTA功能，因此如果第一次烧录kernel，需要烧写oneos的bootloader，目前release的bootloader是 bootloader_with_ble_stack_start_0x0.bin ，直接使用相关工具从 0x0地址烧写就可以。
2. temp文件中放的两个文件，ble_app_hrs_freertos_pca10040_s132_no_boot_kernel_0x0.bin 是原BLE协议栈固件，起始地址为0；boot_nrf52832_0x26000.bin 为 oneos的通用bootloader固件，起始地址为0x26000。为方便后续调试bootloader可以进行相关替换，所以把该bootloader放到了这里。如果需要更新oneos的通用bootloader固件，可以把新的替换到这里。然后分别按相应的地址烧录  ble_app_hrs_freertos_pca10040_s132_no_boot_kernel_0x0.bin 和 boot_nrf52832_0x26000.bin。