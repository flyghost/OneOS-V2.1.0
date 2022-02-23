## NRF Softdevice demos

### Brief

Those three demos 'ble_app_bas.c', 'ble_app_beacon.c', 'ble_app_hrs.c' are based on NRF Softdevice that you can get more information on [official site](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fstruct_nrf52%2Fstruct%2Fnrf52_softdevices.html) for.

### How to use demos with OneOS

1. Enable softdevice. Open menuconfig and enter into the option '(Top) → Drivers→ HAL→ Enable Nordic softdevice', enable this item.  

2. Set target configuration. In keil, you should set 'IROM1' an 'IRAM1' in the 'Options for Target → Target'.  
For 'SOC_NRF52832', 'IROM1 Start' should be set to '0x26000'.  
For 'SOC_NRF52840', 'IROM1 Start' should be set to '0x27000'.
'IRAM1 Start' could be set to '0x20005978' for an initial value as it should be fixed later.

3. Fix 'IRAM1 Start'. Connect Segger JLINK up the board and open J-Link RTT Viewer. Compile the target in keil and start debug.  In the RTT Viewer, you will get the prompt for the start address. Change the 'IRAM1 Start' in keil to this.