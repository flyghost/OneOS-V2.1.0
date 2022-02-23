# WiresharkDump

* Ver: v1.0.0
* Intro: A tool that dumps data from mcu to pc and puts data into Wireshark to be analysed

## Brief

This project is created by me as a component for [OneOS](https://gitee.com/cmcc-oneos/OneOS) to dump data from mcu to pc and analyse data in Wireshark. It can be ported to other RTOS or bare metal system.

## Purpose

Sometimes we need to analyse protocol (as BT/ETH/TCPIP/...) used in Mcu, but it is annoying to analyse protocol data by just printing them out in Hecadecimal format or other format. We are used to analyse protocol in GUI tools that can display protocol data with data bit definition. As Wireshark is a great protocol analysing tool and supports many protocols, I decided to build this project to achieve this goal.

## Structure

```shell
└── wiresharkdump  
    ├── device  
    └── host  
```

### device

Software runs in device that you want to dump data from. It output data you want to dump in some frame format to host (PC).

### host

Software runs in PC. It recieces data from device and transport them to wireshark through the Pipe created in initialization process.

### frame format

|  1B  |  2B  |  4B  |  1B  |  NB  |  1B  |
| ---  | ---  | ---  | ---  | ---  | ---  |
| 0xAA | len  | time | dir  | data | 0x55 |

## How to use 

### device example

This example is based on the demo using OneOS NimBLE stack with Nordic-pca10056 board. To build a project using OneOS Nimble stack you can find help in OneOS Nimble manual.

1. Add the following code to 'components\ble\mynewt-nimble\nimble\transport\ram\src\ble_hci_ram.c'.

```c
#include "nimble/hci_common.h"
#include "wiresharkdump.h"

#define BLE_HCI_UART_H4_NONE 0x00
#define BLE_HCI_UART_H4_CMD 0x01
#define BLE_HCI_UART_H4_ACL 0x02
#define BLE_HCI_UART_H4_SCO 0x03
#define BLE_HCI_UART_H4_EVT 0x04

int
ble_hci_trans_hs_cmd_tx(uint8_t *cmd)
{
    int rc;

    assert(ble_hci_ram_rx_cmd_ll_cb != NULL);

    struct ble_hci_cmd *hci_cmd = (struct ble_hci_cmd *)cmd;
    uint8_t pktlen = sizeof(struct ble_hci_cmd) + hci_cmd->length;
    wsk_bt_hci_paras_t paras = {
        .type = BLE_HCI_UART_H4_CMD,
        .pkt = cmd,
        .len = pktlen};
    wsk_hexdump(wsk_dump_dir_o, wsk_bt_hci_hexdump, &paras);

    rc = ble_hci_ram_rx_cmd_ll_cb(cmd, ble_hci_ram_rx_cmd_ll_arg);
    return rc;
}

int
ble_hci_trans_ll_evt_tx(uint8_t *hci_ev)
{
    int rc;

    assert(ble_hci_ram_rx_cmd_hs_cb != NULL);

    uint16_t pktlen;
    pktlen = 2 + ((struct ble_hci_ev *)hci_ev)->length;
    wsk_bt_hci_paras_t paras = {
        .type = BLE_HCI_UART_H4_EVT,
        .pkt = hci_ev,
        .len = pktlen};
    wsk_hexdump(wsk_dump_dir_o, wsk_bt_hci_hexdump, &paras);

    rc = ble_hci_ram_rx_cmd_hs_cb(hci_ev, ble_hci_ram_rx_cmd_hs_arg);
    return rc;
}
```

2. Add the following code to 'main.c'.

```c
#include "trans-dev.h"
#include "misc_evt.h"
#include "wiresharkdump.h"

int main(int argc, char **argv)
{
    int rc;

    os_device_t *dev = wsk_trans_dev_init("uart0");
    wsk_dump_init(dev);
    misc_evt_init();

    /* Initialize all packages. */
    nimble_port_oneos_init();

    ble_hs_cfg.sync_cb = on_sync;
    ble_hs_cfg.reset_cb = on_reset;

    rc = ble_svc_gap_device_name_set(device_name);
    OS_ASSERT(rc == 0);

    /* As the last thing, process events from default event queue. */
    ble_hs_task_startup();

    while (1)
    {
        os_task_msleep(100);
        if(wsk_dump_sta_check()) {
            os_kprintf("bt_hci_log_overflow\n");
        }
    }

    return 0;
}
```
Note: here we use the 'uart0' which is used as console in origin project to transport frame as 'uart1' is not working properly at present.

3. Enable 'WiresharkDump' component in menuconfig 'Components->Diagnose' item.

### host

### Requirement

* python3
* pyserial
* wireshark
* pywin32 ( Windows )

### Cmd

```shell
python serial_trans.py -h   # Use this cmd to get more information.
```
