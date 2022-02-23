/*
 * @Author: your name
 * @Date: 2021-03-03 14:02:53
 * @LastEditTime: 2021-03-03 17:51:51
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \RK2028Ad:\00-workspace\01-project\smartSocket\hlw8110.h
 */

#ifndef __HLW8110__
#define __HLW8110__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#include "os_clock.h"
#include "os_assert.h"
#include "os_device.h"
#include "serial.h"
#include "os_mailbox.h"
//#include "smart_socket_relay.h"


#define HLW8110_TASCK_SIZE          (1024*1)
#define HLW8110_TASK_PRIORITY       (3)
#define HLW8110_TASK_TICK           (5)

//8112A通道或8110通道校正系数 for testing
#define D_CAL_U		1000/1000		//电压校正系数
#define D_CAL_A_I	1000/1000		//A通道电流校正系数
#define D_CAL_A_P	1000/1000		//A通道功率校正系数
#define D_CAL_A_E	1000/1000		//A通道电能校正系数

#define SMART_SOCKET_UART_HEADER            0xA5

#define SMART_SOCKET_UART_RMS_I_C           0x70
#define SMART_SOCKET_UART_RMS_U_C           0x72
#define SMART_SOCKET_UART_POWER_C           0x73
#define SMART_SOCKET_UART_ENERGY_C          0x76

/* HLW uart configure*/
#define HLW8110_UART_PORT                   "uart2"
#define HLW8110_UART_BAUD                   BAUD_RATE_9600
#define HLW8110_UART_PARITY                 PARITY_EVEN

#define HLW8110_I_CHANNEL_SENSOR_K1         (0.988) /*based on R7*/
#define HLW8110_U_CHANNEL_SENSOR_K2         (1.023) /*based on R13*/

#define HLW8110_W_REG(reg)                  ((reg) | 0x80)
#define HLW8110_R_REG(reg)                  (reg)

/* speical register */
#define HLW8110_SPECIAL_REG                 (0xEA)

/* special register setting value*/
#define HLW8110_CHANNEL_A_ENABLE            (0x0a04)
#define HLW8110_WRITE_ENABLE                (0xE5)
#define HLW8110_WRITE_DISABLE               (0xDC)
#define HLW8110_SELECT_CHANNEL_A            (0x5A)
#define HLW8110_RESET                       (0x96)

/* write register address */
#define HLW8110_SYSCON_REG                  (0x00)
#define HLW8110_EMUCON_REG                  (0x01)
#define HLW8110_HF_CONST_REG                (0x02)
#define HLW8110_EMUCON2_REG                 (0x13)
#define HLW8110_ANGLE_REG                   (0x22)
#define HLW8110_UFREQ_REG                   (0x23)
#define HLW8110_RMS_I_A_REG                 (0x24)
#define HLW8110_RMS_U_REG                   (0x26)
#define HLW8110_PF_REG                      (0x27)
#define HLW8110_ENERGY_PA_REG               (0x28)
#define HLW8110_POWER_PA_REG                (0x2C)

#define HLW8110_REG_IE_ADDR          		(0x40)
#define HLW8110_REG_IF_ADDR          		(0x41)
#define HLW8110_REG_RIF_ADDR          	    (0x42)


#define HLW8110_ALL_FACTOR_CRC_REG          (0x6F)
/* factor register address */
#define HLW8110_RMS_I_A_C                   (0X70)
#define HLW8110_RMS_I_B_C                   (0x71)
#define HLW8110_RMS_U_C                     (0x72)
#define HLW8110_POWER_A_C                   (0x73)
#define HLW8110_POWER_B_C                   (0x74)
#define HLW8110_POWER_SC                    (0x75)
#define HLW8110_ENERGY_A_C                  (0x76)
#define HLW8110_ENERGY_B_C                  (0x77)

/* read register address */


typedef struct {
    os_uint32_t rms_u;
    /* channel A */
    os_uint32_t base_energy_a;
    os_uint32_t bak_energy_a;
    os_uint32_t rms_i_a;
    os_uint32_t energy_a;
    os_uint32_t power_a;
    /* channel B */
    os_uint32_t rms_b_i;
    os_uint32_t energy_b;
    os_uint32_t power_b;

}hlw8110_actual_result_t;

typedef struct {
    os_uint16_t all_factor_reg_crc;
    os_uint16_t rms_u_c;
    os_uint16_t power_sc;
    //channel A
    os_uint16_t rms_i_a_c;
    os_uint16_t energy_a_c;
    os_uint16_t power_a_c;
    //channel B
    os_uint16_t rms_i_b_c;
    os_uint16_t energy_b_c;
    os_uint16_t power_b_c;

}hlw8110_actual_factor_t;

typedef os_err_t (*cb_callback)(os_device_t *dev, struct os_device_cb_info *info);

typedef struct {
    struct os_device *dev;
 
     /* uart TX*/
    cb_callback tx;
    os_uint8_t tx_buf[OS_SERIAL_RX_BUFSZ];
    os_uint16_t tx_bytes;
    os_mailbox_t tx_mb; 
    os_sem_t tx_sem;

    /* uart RX */
    cb_callback rx;
    os_uint16_t rx_bytes;
    os_uint8_t rx_buf[OS_SERIAL_RX_BUFSZ];
    os_mailbox_t rx_mb;
    os_sem_t rx_sem;

}gc_uart_t;

extern int hlw8110_init(os_uint32_t energy_a);

extern void* gc_serial_open(void);
extern int gc_serial_close(void *dev);
extern int gc_serial_read(void * pbuf, os_uint16_t len);
extern int gc_serial_write(void * pbuf, os_uint16_t len);
extern hlw8110_actual_result_t get_hlw8110_measurements(void);

#ifdef __cplusplus
}
#endif

#endif /*__HLW8110__*/