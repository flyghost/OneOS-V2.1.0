#include <drv_cfg.h>
#include <arch_interrupt.h>
#include <device.h>
#include <os_errno.h>
#include <os_clock.h>
#include <os_util.h>
#include <os_memory.h>
#include <string.h>
#include <stdio.h>
#include <drv_gpio.h>
#include <sensors/sensor.h>

#define DBG_TAG "sensor.adi.lsm6dso"
#include <dlog.h>


/** I2C Device Address 8 bit format  if SA0=0 -> D5 if SA0=1 -> D7 **/
#define LSM6DSO_I2C_ADD_L                    0xD5
#define LSM6DSO_I2C_ADD_H                    0xD7

/** Device Identification (Who am I) **/
#define LSM6DSO_ID                           0x6C

/**
  * @}
  *
  */

#define LSM6DSO_FUNC_CFG_ACCESS              0x01U
#define LSM6DSO_PIN_CTRL                     0x02U
#define LSM6DSO_FIFO_CTRL1                   0x07U
#define LSM6DSO_FIFO_CTRL2                   0x08U
#define LSM6DSO_FIFO_CTRL3                   0x09U
#define LSM6DSO_FIFO_CTRL4                   0x0AU
#define LSM6DSO_COUNTER_BDR_REG1             0x0BU
#define LSM6DSO_COUNTER_BDR_REG2             0x0CU
#define LSM6DSO_INT1_CTRL                    0x0D
#define LSM6DSO_INT2_CTRL                    0x0EU
#define LSM6DSO_WHO_AM_I                     0x0FU
#define LSM6DSO_CTRL1_XL                     0x10U
#define LSM6DSO_CTRL2_G                      0x11U
#define LSM6DSO_CTRL3_C                      0x12U
#define LSM6DSO_CTRL4_C                      0x13U
#define LSM6DSO_CTRL5_C                      0x14U
#define LSM6DSO_CTRL6_C                      0x15U
#define LSM6DSO_CTRL7_G                      0x16U
#define LSM6DSO_CTRL8_XL                     0x17U
#define LSM6DSO_CTRL9_XL                     0x18U
#define LSM6DSO_CTRL10_C                     0x19U
#define LSM6DSO_ALL_INT_SRC                  0x1AU
#define LSM6DSO_WAKE_UP_SRC                  0x1BU
#define LSM6DSO_TAP_SRC                      0x1CU
#define LSM6DSO_D6D_SRC                      0x1DU
#define LSM6DSO_STATUS_REG                   0x1EU
#define LSM6DSO_STATUS_SPIAUX                0x1EU
#define LSM6DSO_OUT_TEMP_L                   0x20U
#define LSM6DSO_OUT_TEMP_H                   0x21U
#define LSM6DSO_OUTX_L_G                     0x22U
#define LSM6DSO_OUTX_H_G                     0x23U
#define LSM6DSO_OUTY_L_G                     0x24U
#define LSM6DSO_OUTY_H_G                     0x25U
#define LSM6DSO_OUTZ_L_G                     0x26U
#define LSM6DSO_OUTZ_H_G                     0x27U
#define LSM6DSO_OUTX_L_A                     0x28U
#define LSM6DSO_OUTX_H_A                     0x29U
#define LSM6DSO_OUTY_L_A                     0x2AU
#define LSM6DSO_OUTY_H_A                     0x2BU
#define LSM6DSO_OUTZ_L_A                     0x2CU
#define LSM6DSO_OUTZ_H_A                     0x2DU
#define LSM6DSO_EMB_FUNC_STATUS_MAINPAGE     0x35U
#define LSM6DSO_FSM_STATUS_A_MAINPAGE        0x36U
#define LSM6DSO_FSM_STATUS_B_MAINPAGE        0x37U
#define LSM6DSO_STATUS_MASTER_MAINPAGE       0x39U
#define LSM6DSO_FIFO_STATUS1                 0x3AU
#define LSM6DSO_FIFO_STATUS2                 0x3B
#define LSM6DSO_TIMESTAMP0                   0x40U
#define LSM6DSO_TIMESTAMP1                   0x41U
#define LSM6DSO_TIMESTAMP2                   0x42U
#define LSM6DSO_TIMESTAMP3                   0x43U
#define LSM6DSO_TAP_CFG0                     0x56U
#define LSM6DSO_TAP_CFG1                     0x57U
#define LSM6DSO_TAP_CFG2                     0x58U
#define LSM6DSO_TAP_THS_6D                   0x59U
#define LSM6DSO_INT_DUR2                     0x5AU
#define LSM6DSO_WAKE_UP_THS                  0x5BU
#define LSM6DSO_WAKE_UP_DUR                  0x5CU
#define LSM6DSO_FREE_FALL                    0x5DU
#define LSM6DSO_MD1_CFG                      0x5EU
#define LSM6DSO_MD2_CFG                      0x5FU
#define LSM6DSO_I3C_BUS_AVB                  0x62U
#define LSM6DSO_INTERNAL_FREQ_FINE           0x63U
#define LSM6DSO_INT_OIS                      0x6FU
#define LSM6DSO_CTRL1_OIS                    0x70U
#define LSM6DSO_CTRL2_OIS                    0x71U
#define LSM6DSO_CTRL3_OIS                    0x72U
#define LSM6DSO_X_OFS_USR                    0x73U
#define LSM6DSO_Y_OFS_USR                    0x74U
#define LSM6DSO_Z_OFS_USR                    0x75U
#define LSM6DSO_FIFO_DATA_OUT_TAG            0x78U
#define LSM6DSO_FIFO_DATA_OUT_X_L            0x79U
#define LSM6DSO_FIFO_DATA_OUT_X_H            0x7AU
#define LSM6DSO_FIFO_DATA_OUT_Y_L            0x7BU
#define LSM6DSO_FIFO_DATA_OUT_Y_H            0x7CU
#define LSM6DSO_FIFO_DATA_OUT_Z_L            0x7DU
#define LSM6DSO_FIFO_DATA_OUT_Z_H            0x7EU
#define LSM6DSO_PAGE_SEL                     0x02U
#define LSM6DSO_EMB_FUNC_EN_A                0x04U
#define LSM6DSO_EMB_FUNC_EN_B                0x05U
#define LSM6DSO_PAGE_ADDRESS                 0x08U
#define LSM6DSO_PAGE_VALUE                   0x09U
#define LSM6DSO_EMB_FUNC_INT1                0x0AU
#define LSM6DSO_FSM_INT1_A                   0x0BU
#define LSM6DSO_FSM_INT1_B                   0x0CU
#define LSM6DSO_EMB_FUNC_INT2                0x0EU
#define LSM6DSO_FSM_INT2_A                   0x0FU
#define LSM6DSO_FSM_INT2_B                   0x10U
#define LSM6DSO_EMB_FUNC_STATUS              0x12U
#define LSM6DSO_FSM_STATUS_A                 0x13U
#define LSM6DSO_FSM_STATUS_B                 0x14U
#define LSM6DSO_PAGE_RW                      0x17U
#define LSM6DSO_EMB_FUNC_FIFO_CFG            0x44U
#define LSM6DSO_FSM_ENABLE_A                 0x46U
#define LSM6DSO_FSM_ENABLE_B                 0x47U
#define LSM6DSO_FSM_LONG_COUNTER_L           0x48U
#define LSM6DSO_FSM_LONG_COUNTER_H           0x49U
#define LSM6DSO_FSM_LONG_COUNTER_CLEAR       0x4AU
#define LSM6DSO_FSM_OUTS1                    0x4CU
#define LSM6DSO_FSM_OUTS2                    0x4DU
#define LSM6DSO_FSM_OUTS3                    0x4EU
#define LSM6DSO_FSM_OUTS4                    0x4FU
#define LSM6DSO_FSM_OUTS5                    0x50U
#define LSM6DSO_FSM_OUTS6                    0x51U
#define LSM6DSO_FSM_OUTS7                    0x52U
#define LSM6DSO_FSM_OUTS8                    0x53U
#define LSM6DSO_FSM_OUTS9                    0x54U
#define LSM6DSO_FSM_OUTS10                   0x55U
#define LSM6DSO_FSM_OUTS11                   0x56U
#define LSM6DSO_FSM_OUTS12                   0x57U
#define LSM6DSO_FSM_OUTS13                   0x58U
#define LSM6DSO_FSM_OUTS14                   0x59U
#define LSM6DSO_FSM_OUTS15                   0x5AU
#define LSM6DSO_FSM_OUTS16                   0x5BU
#define LSM6DSO_EMB_FUNC_ODR_CFG_B           0x5FU
#define LSM6DSO_STEP_COUNTER_L               0x62U
#define LSM6DSO_STEP_COUNTER_H               0x63U
#define LSM6DSO_EMB_FUNC_SRC                 0x64U
#define LSM6DSO_EMB_FUNC_INIT_A              0x66U
#define LSM6DSO_EMB_FUNC_INIT_B              0x67U
#define LSM6DSO_MAG_SENSITIVITY_L            0xBAU
#define LSM6DSO_MAG_SENSITIVITY_H            0xBBU
#define LSM6DSO_MAG_OFFX_L                   0xC0U
#define LSM6DSO_MAG_OFFX_H                   0xC1U
#define LSM6DSO_MAG_OFFY_L                   0xC2U
#define LSM6DSO_MAG_OFFY_H                   0xC3U
#define LSM6DSO_MAG_OFFZ_L                   0xC4U
#define LSM6DSO_MAG_OFFZ_H                   0xC5U
#define LSM6DSO_MAG_SI_XX_L                  0xC6U
#define LSM6DSO_MAG_SI_XX_H                  0xC7U
#define LSM6DSO_MAG_SI_XY_L                  0xC8U
#define LSM6DSO_MAG_SI_XY_H                  0xC9U
#define LSM6DSO_MAG_SI_XZ_L                  0xCAU
#define LSM6DSO_MAG_SI_XZ_H                  0xCBU
#define LSM6DSO_MAG_SI_YY_L                  0xCCU
#define LSM6DSO_MAG_SI_YY_H                  0xCDU
#define LSM6DSO_MAG_SI_YZ_L                  0xCEU
#define LSM6DSO_MAG_SI_YZ_H                  0xCFU
#define LSM6DSO_MAG_SI_ZZ_L                  0xD0U
#define LSM6DSO_MAG_SI_ZZ_H                  0xD1U
#define LSM6DSO_MAG_CFG_A                    0xD4U
#define LSM6DSO_MAG_CFG_B                    0xD5U
#define LSM6DSO_FSM_LC_TIMEOUT_L             0x17AU
#define LSM6DSO_FSM_LC_TIMEOUT_H             0x17BU
#define LSM6DSO_FSM_PROGRAMS                 0x17CU
#define LSM6DSO_FSM_START_ADD_L              0x17EU
#define LSM6DSO_FSM_START_ADD_H              0x17FU
#define LSM6DSO_PEDO_CMD_REG                 0x183U
#define LSM6DSO_PEDO_DEB_STEPS_CONF          0x184U
#define LSM6DSO_PEDO_SC_DELTAT_L             0x1D0U
#define LSM6DSO_PEDO_SC_DELTAT_H             0x1D1U
#define LSM6DSO_SENSOR_HUB_1                 0x02U
#define LSM6DSO_SENSOR_HUB_2                 0x03U
#define LSM6DSO_SENSOR_HUB_3                 0x04U
#define LSM6DSO_SENSOR_HUB_4                 0x05U
#define LSM6DSO_SENSOR_HUB_5                 0x06U
#define LSM6DSO_SENSOR_HUB_6                 0x07U
#define LSM6DSO_SENSOR_HUB_7                 0x08U
#define LSM6DSO_SENSOR_HUB_8                 0x09U
#define LSM6DSO_SENSOR_HUB_9                 0x0AU
#define LSM6DSO_SENSOR_HUB_10                0x0BU
#define LSM6DSO_SENSOR_HUB_11                0x0CU
#define LSM6DSO_SENSOR_HUB_12                0x0DU
#define LSM6DSO_SENSOR_HUB_13                0x0EU
#define LSM6DSO_SENSOR_HUB_14                0x0FU
#define LSM6DSO_SENSOR_HUB_15                0x10U
#define LSM6DSO_SENSOR_HUB_16                0x11U
#define LSM6DSO_SENSOR_HUB_17                0x12U
#define LSM6DSO_SENSOR_HUB_18                0x13U
#define LSM6DSO_MASTER_CONFIG                0x14U
#define LSM6DSO_SLV0_ADD                     0x15U
#define LSM6DSO_SLV0_SUBADD                  0x16U
#define LSM6DSO_SLV0_CONFIG                  0x17U
#define LSM6DSO_SLV1_ADD                     0x18U
#define LSM6DSO_SLV1_SUBADD                  0x19U
#define LSM6DSO_SLV1_CONFIG                  0x1AU
#define LSM6DSO_SLV2_ADD                     0x1BU
#define LSM6DSO_SLV2_SUBADD                  0x1CU
#define LSM6DSO_SLV2_CONFIG                  0x1DU
#define LSM6DSO_SLV3_ADD                     0x1EU
#define LSM6DSO_SLV3_SUBADD                  0x1FU
#define LSM6DSO_SLV3_CONFIG                  0x20U
#define LSM6DSO_DATAWRITE_SLV0               0x21U
#define LSM6DSO_STATUS_MASTER                0x22U
#define LSM6DSO_START_FSM_ADD                0x0400U


typedef enum {
  LSM6DSO_2g   = 0,
  LSM6DSO_16g  = 1, /* if XL_FS_MODE = ¡®1¡¯ -> LSM6DSO_2g */
  LSM6DSO_4g   = 2,
  LSM6DSO_8g   = 3,
} lsm6dso_fs_xl_t;

#define LSM6DSO_ACC_SENSITIVITY_FS_2G   0.061f
#define LSM6DSO_ACC_SENSITIVITY_FS_4G   0.122f
#define LSM6DSO_ACC_SENSITIVITY_FS_8G   0.244f
#define LSM6DSO_ACC_SENSITIVITY_FS_16G  0.488f

typedef enum {
  LSM6DSO_250dps   = 0,
  LSM6DSO_125dps   = 1,
  LSM6DSO_500dps   = 2,
  LSM6DSO_1000dps  = 4,
  LSM6DSO_2000dps  = 6,
} lsm6dso_fs_g_t;
  
#define LSM6DSO_GYRO_SENSITIVITY_FS_125DPS    4.375f
#define LSM6DSO_GYRO_SENSITIVITY_FS_250DPS    8.750f
#define LSM6DSO_GYRO_SENSITIVITY_FS_500DPS   17.500f
#define LSM6DSO_GYRO_SENSITIVITY_FS_1000DPS  35.000f
#define LSM6DSO_GYRO_SENSITIVITY_FS_2000DPS  70.000f
typedef enum {
  LSM6DSO_XL_ODR_OFF    = 0,
  LSM6DSO_XL_ODR_12Hz5  = 1,
  LSM6DSO_XL_ODR_26Hz   = 2,
  LSM6DSO_XL_ODR_52Hz   = 3,
  LSM6DSO_XL_ODR_104Hz  = 4,
  LSM6DSO_XL_ODR_208Hz  = 5,
  LSM6DSO_XL_ODR_417Hz  = 6,
  LSM6DSO_XL_ODR_833Hz  = 7,
  LSM6DSO_XL_ODR_1667Hz = 8,
  LSM6DSO_XL_ODR_3333Hz = 9,
  LSM6DSO_XL_ODR_6667Hz = 10,
  LSM6DSO_XL_ODR_1Hz6   = 11, /* (low power only) */
} lsm6dso_odr_xl_t;


typedef enum {
  LSM6DSO_GY_ODR_OFF    = 0,
  LSM6DSO_GY_ODR_12Hz5  = 1,
  LSM6DSO_GY_ODR_26Hz   = 2,
  LSM6DSO_GY_ODR_52Hz   = 3,
  LSM6DSO_GY_ODR_104Hz  = 4,
  LSM6DSO_GY_ODR_208Hz  = 5,
  LSM6DSO_GY_ODR_417Hz  = 6,
  LSM6DSO_GY_ODR_833Hz  = 7,
  LSM6DSO_GY_ODR_1667Hz = 8,
  LSM6DSO_GY_ODR_3333Hz = 9,
  LSM6DSO_GY_ODR_6667Hz = 10,
} lsm6dso_odr_g_t;


typedef struct
{
  struct os_sensor_device sensor;
  struct os_i2c_client    i2c;
  os_uint8_t              id;

  union
  {
      struct
      {
          short x;
          short y;
          short z;
      } acce;

      struct
      {
          short x;
          short y;
          short z;
      } gyro;
  } value;
} lsm6dso_info_t;

static os_err_t lsm6dso_write(lsm6dso_info_t *lsm6dso, uint8_t Reg, uint8_t Value)
{
    return os_i2c_client_write_byte(&lsm6dso->i2c, Reg, 1, Value);
}

static uint8_t lsm6dso_read(lsm6dso_info_t *lsm6dso, uint8_t Reg)
{
    return os_i2c_client_read_byte(&lsm6dso->i2c, Reg, 1);
}

static os_err_t lsm6dso_read_buff(lsm6dso_info_t *lsm6dso, uint8_t Reg, uint8_t *Buffer, uint16_t Length)
{
    return os_i2c_client_read(&lsm6dso->i2c, Reg, 1, Buffer, Length);
}


void lsm6dso_acc_init(lsm6dso_info_t *lsm6dso, uint8_t InitStruct)
{
    uint8_t tmp;
    uint8_t val[2]={0};
    uint8_t cfg_b = 0;
    uint8_t odr_xl = 0;
    uint8_t fs_xl = 0;
    /* Enable access to the embedded functions/sensor  */
    tmp = lsm6dso_read(lsm6dso, LSM6DSO_FUNC_CFG_ACCESS);
    tmp |= 0x80;
    lsm6dso_write(lsm6dso,LSM6DSO_FUNC_CFG_ACCESS,tmp);
    
    lsm6dso_read_buff(lsm6dso, LSM6DSO_FSM_ENABLE_A, (uint8_t*) val, 2);
    
    tmp = lsm6dso_read(lsm6dso, LSM6DSO_FUNC_CFG_ACCESS);
    tmp &= 0x3F;
    lsm6dso_write(lsm6dso,LSM6DSO_FUNC_CFG_ACCESS,tmp);

    tmp = lsm6dso_read(lsm6dso, LSM6DSO_FUNC_CFG_ACCESS);
    tmp &= 0xBF;
    lsm6dso_write(lsm6dso,LSM6DSO_FUNC_CFG_ACCESS,tmp);
    
    tmp = lsm6dso_read(lsm6dso, LSM6DSO_EMB_FUNC_ODR_CFG_B);
    cfg_b = (tmp&0x18)>>3;
    
    tmp = lsm6dso_read(lsm6dso, LSM6DSO_FUNC_CFG_ACCESS);
    tmp &= 0x3F;
    lsm6dso_write(lsm6dso,LSM6DSO_FUNC_CFG_ACCESS,tmp);
    if ((InitStruct>>4)< cfg_b )
    {
        odr_xl = cfg_b + 1;
    }
    else
    {
        odr_xl = (InitStruct>>4);
    }
    /* Write value to ACC MEMS CTRL1_XL register: FS and Data Rate */
    tmp = lsm6dso_read(lsm6dso, LSM6DSO_CTRL1_XL);
    tmp = ((tmp&0x0F)|(odr_xl<<4));
    lsm6dso_write(lsm6dso, LSM6DSO_CTRL1_XL, tmp);
    
    fs_xl = (InitStruct & 0x03);
    tmp = lsm6dso_read(lsm6dso, LSM6DSO_CTRL1_XL);
    tmp = ((tmp&0xF3)|(fs_xl<<2));
    lsm6dso_write(lsm6dso, LSM6DSO_CTRL1_XL, tmp);
//    tmp = lsm6dso_read(lsm6dso, LSM6DSO_CTRL1_XL);
//    os_kprintf("LSM6DSO_CTRL1_XL gcc data:0x%x\r\n",tmp);
}

void lsm6dso_acc_deinit(lsm6dso_info_t *lsm6dso)
{
    uint8_t tmp;
    uint8_t val[2]={0};
    uint8_t cfg_b = 0;
    uint8_t odr_xl = 0;

    tmp = lsm6dso_read(lsm6dso, LSM6DSO_FUNC_CFG_ACCESS);
    tmp &= 0xBF;
    lsm6dso_write(lsm6dso,LSM6DSO_FUNC_CFG_ACCESS,tmp);
    
    tmp = lsm6dso_read(lsm6dso, LSM6DSO_EMB_FUNC_ODR_CFG_B);
    cfg_b = (tmp&0x18)>>3;
    
    tmp = lsm6dso_read(lsm6dso, LSM6DSO_FUNC_CFG_ACCESS);
    tmp &= 0x3F;
    lsm6dso_write(lsm6dso,LSM6DSO_FUNC_CFG_ACCESS,tmp);

    odr_xl = cfg_b + 1;
    
    /* Write value to ACC MEMS CTRL1_XL register: FS and Data Rate */
    tmp = lsm6dso_read(lsm6dso, LSM6DSO_CTRL1_XL);
    tmp &= ((odr_xl<<4)|0x0F);
    lsm6dso_write(lsm6dso, LSM6DSO_CTRL1_XL, tmp);


}

uint8_t lsm6dso_acc_read_id(lsm6dso_info_t *lsm6dso)
{
    /* Read value at Who am I register address */
    return lsm6dso_read(lsm6dso, LSM6DSO_WHO_AM_I);
}

typedef union{
  int16_t i16bit[3];
  uint8_t u8bit[6];
} lsm6dso_axis3bit16_t;

void lsm6dso_acc_get_value(lsm6dso_info_t *lsm6dso, int16_t *pData)
{
    uint8_t ctrlx = 0;
    uint8_t i           = 0;
    float   sensitivity = 0;
    lsm6dso_axis3bit16_t data_raw;
    
    lsm6dso_read_buff(lsm6dso, LSM6DSO_OUTX_L_A, data_raw.u8bit, 6);

    /* Read the acceleration control register content */
    ctrlx = lsm6dso_read(lsm6dso, LSM6DSO_CTRL1_XL);

    /* Switch the sensitivity value set in the CRTL1_XL */
    switch ((ctrlx>>2) & 0x03)
    {
    case LSM6DSO_2g:
        sensitivity = LSM6DSO_ACC_SENSITIVITY_FS_2G;
        break;
    case LSM6DSO_16g:
        sensitivity = LSM6DSO_ACC_SENSITIVITY_FS_16G;
        break;
    case LSM6DSO_4g:
        sensitivity = LSM6DSO_ACC_SENSITIVITY_FS_4G;
        break;
    case LSM6DSO_8g:
        sensitivity = LSM6DSO_ACC_SENSITIVITY_FS_8G;
        break;
    }

    /* Obtain the mg value for the three axis */
    for (i = 0; i < 3; i++)
    {
        pData[i] = ((int16_t)data_raw.i16bit[i] * sensitivity);
    }
}

void lsm6dso_gyro_low_power(lsm6dso_info_t *lsm6dso, uint16_t status)
{

}

void lsm6dso_gyro_init(lsm6dso_info_t *lsm6dso, uint8_t InitStruct)
{
    uint8_t tmp;
    uint8_t odr_gy = 0;
    uint8_t fs_g = 0;
    uint8_t val[2]={0};

    tmp = lsm6dso_read(lsm6dso, LSM6DSO_FUNC_CFG_ACCESS);
    tmp |= 0x80;
    lsm6dso_write(lsm6dso,LSM6DSO_FUNC_CFG_ACCESS,tmp);
    
    lsm6dso_read_buff(lsm6dso, LSM6DSO_FSM_ENABLE_A, (uint8_t*) val, 2);
    
    tmp = lsm6dso_read(lsm6dso, LSM6DSO_FUNC_CFG_ACCESS);
    tmp &= 0x3F;
    lsm6dso_write(lsm6dso,LSM6DSO_FUNC_CFG_ACCESS,tmp);
    
    tmp = lsm6dso_read(lsm6dso, LSM6DSO_EMB_FUNC_ODR_CFG_B);
    odr_gy = (tmp&0x18)>>3;
    
    if ((InitStruct>>4)< odr_gy )
    {
        odr_gy = odr_gy + 1;
    }
    else
    {
        odr_gy = (InitStruct>>4);
    }
    tmp = lsm6dso_read(lsm6dso, LSM6DSO_CTRL2_G);
    tmp = ((tmp&0x0F)|(odr_gy<<4));
    lsm6dso_write(lsm6dso, LSM6DSO_CTRL2_G, tmp);
    
    fs_g = (InitStruct & 0x07);
    tmp = lsm6dso_read(lsm6dso, LSM6DSO_CTRL2_G);
    tmp = ((tmp&0xF1)|(fs_g<<1));
    lsm6dso_write(lsm6dso, LSM6DSO_CTRL2_G, tmp);
    
//    tmp = lsm6dso_read(lsm6dso, LSM6DSO_CTRL2_G);
//    os_kprintf("LSM6DSO_CTRL2_G gyro data:0x%x\r\n",tmp);
}

void lsm6dso_gyro_deinit(lsm6dso_info_t *lsm6dso)
{
    uint8_t tmp;
    uint8_t odr_gy = 0;
    uint8_t fs_g = 0;


    tmp = lsm6dso_read(lsm6dso, LSM6DSO_FUNC_CFG_ACCESS);
    tmp &= 0xBF;
    lsm6dso_write(lsm6dso,LSM6DSO_FUNC_CFG_ACCESS,tmp);
    
    tmp = lsm6dso_read(lsm6dso, LSM6DSO_EMB_FUNC_ODR_CFG_B);
    fs_g = (tmp&0x18)>>3;
    
    tmp = lsm6dso_read(lsm6dso, LSM6DSO_FUNC_CFG_ACCESS);
    tmp &= 0x3F;
    lsm6dso_write(lsm6dso,LSM6DSO_FUNC_CFG_ACCESS,tmp);
    odr_gy = fs_g + 1;
    tmp = lsm6dso_read(lsm6dso, LSM6DSO_CTRL2_G);
    tmp &= ((odr_gy<<4)|0x0F);
    lsm6dso_write(lsm6dso, LSM6DSO_CTRL2_G, tmp);

}

uint8_t lsm6dso_gyro_read_id(lsm6dso_info_t *lsm6dso)
{
    /* IO interface initialization */
    /* Read value at Who am I register address */
    return lsm6dso_read(lsm6dso, LSM6DSO_WHO_AM_I);

}

void lsm6dso_gyro_get_value(lsm6dso_info_t *lsm6dso, float *pfData)
{
    uint8_t ctrlg = 0;
    uint8_t i           = 0;
    float   sensitivity = 0;
    uint8_t tmp=0;
    lsm6dso_axis3bit16_t data_raw;
    
    /* Read output register X, Y & Z acceleration */
    lsm6dso_read_buff(lsm6dso, LSM6DSO_OUTX_L_G, data_raw.u8bit, 6);
    
    /* Read the gyro control register content */
    ctrlg = lsm6dso_read(lsm6dso, LSM6DSO_CTRL2_G);

    /* Normal mode */
    /* Switch the sensitivity value set in the CRTL2_G */
    switch ((ctrlg & 0x0E)>>1)
    {
        case LSM6DSO_125dps:
          sensitivity = LSM6DSO_GYRO_SENSITIVITY_FS_125DPS;
          break;
        
        case LSM6DSO_250dps:
          sensitivity = LSM6DSO_GYRO_SENSITIVITY_FS_250DPS;
          break;
        
        case LSM6DSO_500dps:
          sensitivity = LSM6DSO_GYRO_SENSITIVITY_FS_500DPS;
          break;
        
        case LSM6DSO_1000dps:
          sensitivity = LSM6DSO_GYRO_SENSITIVITY_FS_1000DPS;
          break;
        
        case LSM6DSO_2000dps:
          sensitivity = LSM6DSO_GYRO_SENSITIVITY_FS_2000DPS;
          break;

    }

    for (i = 0; i < 3; i++)
    {
        pfData[i] = (float)data_raw.i16bit[i] / (float)sensitivity;
    }
}


#define LSM6DSO_I3C_DISABLE  0x80
static lsm6dso_info_t *lsm6dso_sensor_init(const char *bus_name, os_uint16_t addr, int acc_or_gyro)
{
    unsigned char   devid   = 0;
    lsm6dso_info_t *lsm6dso = NULL;
    static uint8_t init_state = 0;
    uint8_t InitStruct = 0;
    LOG_I(DBG_TAG,"lsm6dso:[%s][0x%02x]", bus_name, addr);

    lsm6dso = os_calloc(1, sizeof(lsm6dso_info_t));
    if (lsm6dso == OS_NULL)
    {
        LOG_E(DBG_TAG,"lsm6dso amlloc faile");
        return NULL;
    }

    lsm6dso->i2c.bus = os_i2c_bus_device_find(bus_name);
    if (lsm6dso->i2c.bus == NULL)
    {
        LOG_E(DBG_TAG,"lsm6dso i2c invalid.");
        os_free(lsm6dso);
        return NULL;
    }

    lsm6dso->i2c.client_addr = OS_LSM6DSO_I2C_ADDR;

    devid = lsm6dso_read(lsm6dso, LSM6DSO_WHO_AM_I);

    LOG_I(DBG_TAG,"lsm6dso devid:0x%02x", devid);
    if (devid != LSM6DSO_ID)
    {
        LOG_E(DBG_TAG,"lsm6dso devid invalid.");
        os_free(lsm6dso);
        return NULL;
    }
    else
    {
        if(init_state == 0)
        {
            /* Disable I3C */
            unsigned char ctrl9_xl = lsm6dso_read(lsm6dso, LSM6DSO_CTRL9_XL);
            ctrl9_xl |= (0x02);
            lsm6dso_write(lsm6dso,LSM6DSO_CTRL9_XL,ctrl9_xl);
            
             unsigned char i3c_bus_avb = lsm6dso_read(lsm6dso, LSM6DSO_I3C_BUS_AVB);
             i3c_bus_avb &= 0xCF;
             lsm6dso_write(lsm6dso, LSM6DSO_I3C_BUS_AVB,i3c_bus_avb);

             /* Enable register address automatically incremented during a multiple byte
             access with a serial interface. */
            unsigned char ctrl3_c = lsm6dso_read(lsm6dso, LSM6DSO_CTRL3_C);
            ctrl3_c |= (0x04);
            lsm6dso_write(lsm6dso,LSM6DSO_CTRL3_C,ctrl3_c);
            
            /* Enable BDU */
            ctrl3_c = lsm6dso_read(lsm6dso, LSM6DSO_CTRL3_C);
            ctrl3_c |= (0x40);
            lsm6dso_write(lsm6dso,LSM6DSO_CTRL3_C,ctrl3_c);
            
            /* FIFO mode selection */
            unsigned char ctrl4_t = lsm6dso_read(lsm6dso, LSM6DSO_FIFO_CTRL4);
            ctrl4_t &= (0xf8);
            lsm6dso_write(lsm6dso,LSM6DSO_FIFO_CTRL4,ctrl4_t);
            init_state = 1;
        }
    }

    lsm6dso->id = devid;



    if (acc_or_gyro)
    {
        InitStruct = ((LSM6DSO_XL_ODR_104Hz<<4) 
                   | LSM6DSO_2g);
        lsm6dso_acc_init(lsm6dso, InitStruct);
    }
    else
    {
        InitStruct = (LSM6DSO_GY_ODR_104Hz <<4)
                   | LSM6DSO_2000dps;
        lsm6dso_gyro_init(lsm6dso, InitStruct);
    }

    return lsm6dso;
}

static uint8_t lsm6dso_acce_sensor_get_value(lsm6dso_info_t *lsm6dso)
{
    int16_t pData[3];

    lsm6dso_acc_get_value(lsm6dso, pData);

    lsm6dso->value.acce.x = pData[0];
    lsm6dso->value.acce.y = pData[1];
    lsm6dso->value.acce.z = pData[2];

    LOG_D(DBG_TAG,"(%d, %d, %d).", (int)lsm6dso->value.acce.x, (int)lsm6dso->value.acce.y, (int)lsm6dso->value.acce.z);

    return 0;
}

static os_size_t lsm6dso_acce_sensor_fetch_data(struct os_sensor_device *sensor, void *buf, os_size_t len)
{
    lsm6dso_info_t *lsm6dso = NULL;
    struct os_sensor_data *data    = NULL;

    OS_ASSERT(sensor);
    OS_ASSERT(sensor->info.type == OS_SENSOR_CLASS_ACCE);
    OS_ASSERT(buf);

    lsm6dso = (lsm6dso_info_t *)sensor;
    data    = (struct os_sensor_data *)buf;

    lsm6dso_acce_sensor_get_value(lsm6dso);

    data->type        = sensor->info.type;
    data->data.acce.x = lsm6dso->value.acce.x;
    data->data.acce.y = lsm6dso->value.acce.y;
    data->data.acce.z = lsm6dso->value.acce.z;
    data->timestamp   = os_sensor_get_ts();

    return 0;
}

static os_err_t lsm6dso_acce_sensor_control(struct os_sensor_device *sensor, int cmd, void *args)
{
    lsm6dso_info_t *lsm6dso = (lsm6dso_info_t *)sensor;
    os_err_t        result  = OS_EOK;

    switch (cmd)
    {
    case OS_SENSOR_CTRL_GET_ID:
        *(uint8_t *)args = lsm6dso->id;
        break;
    default:
        return OS_ERROR;
    }
    return result;
}

static struct os_sensor_ops lsm6dso_acce_ops =
{
    lsm6dso_acce_sensor_fetch_data,
    lsm6dso_acce_sensor_control
};

static int os_hw_lsm6dso_acce_init(void)
{
    os_int8_t       result;
    lsm6dso_info_t *lsm6dso = NULL;

    lsm6dso = lsm6dso_sensor_init(OS_LSM6DSO_I2C_BUS_NAME, OS_LSM6DSO_I2C_ADDR, 1);
    if (lsm6dso == NULL)
    {
        LOG_E(DBG_TAG,"lsm6dso init failed.");
        goto __exit;
    }

    lsm6dso->sensor.info.type       = OS_SENSOR_CLASS_ACCE;
    lsm6dso->sensor.info.vendor     = OS_SENSOR_VENDOR_INVENSENSE;
    lsm6dso->sensor.info.model      = "lsm6dso";
    lsm6dso->sensor.info.unit       = OS_SENSOR_UNIT_MG;
    lsm6dso->sensor.info.intf_type  = OS_SENSOR_INTF_I2C;
    lsm6dso->sensor.info.range_max  = 16000;
    lsm6dso->sensor.info.range_min  = -16000;
    lsm6dso->sensor.info.period_min = 5;
    lsm6dso->sensor.ops             = &lsm6dso_acce_ops;

    result = os_hw_sensor_register(&lsm6dso->sensor, "lsm6dso", OS_NULL);
    if (result != OS_EOK)
    {
        LOG_E(DBG_TAG,"device register err code: %d", result);
        goto __exit;
    }

    LOG_I(DBG_TAG,"lsm6dso init success");
    return OS_EOK;

__exit:
    if (lsm6dso)
    {
        os_free(lsm6dso);
    }
    return OS_ERROR;
}

OS_DEVICE_INIT(os_hw_lsm6dso_acce_init,OS_INIT_SUBLEVEL_LOW);

static uint8_t lsm6dso_gyro_sensor_get_value(lsm6dso_info_t *lsm6dso)
{
    float pData[3];

    lsm6dso_gyro_get_value(lsm6dso, pData);

    lsm6dso->value.gyro.x = pData[0];
    lsm6dso->value.gyro.y = pData[1];
    lsm6dso->value.gyro.z = pData[2];

    LOG_D(DBG_TAG,"(%d, %d, %d).", (int)lsm6dso->value.gyro.x, (int)lsm6dso->value.gyro.y, (int)lsm6dso->value.gyro.z);

    return 0;
}

static os_size_t lsm6dso_gyro_sensor_fetch_data(struct os_sensor_device *sensor, void *buf, os_size_t len)
{
    lsm6dso_info_t *lsm6dso = NULL;
    struct os_sensor_data *data    = NULL;

    OS_ASSERT(sensor);
    OS_ASSERT(sensor->info.type == OS_SENSOR_CLASS_GYRO);
    OS_ASSERT(buf);

    lsm6dso = (lsm6dso_info_t *)sensor;
    data    = (struct os_sensor_data *)buf;

    lsm6dso_gyro_sensor_get_value(lsm6dso);

    data->type        = sensor->info.type;
    data->data.acce.x = lsm6dso->value.gyro.x;
    data->data.acce.y = lsm6dso->value.gyro.y;
    data->data.acce.z = lsm6dso->value.gyro.z;
    data->timestamp   = os_sensor_get_ts();

    return 0;
}

static os_err_t lsm6dso_gyro_sensor_control(struct os_sensor_device *sensor, int cmd, void *args)
{
    lsm6dso_info_t *lsm6dso = (lsm6dso_info_t *)sensor;
    os_err_t        result  = OS_EOK;

    switch (cmd)
    {
    case OS_SENSOR_CTRL_GET_ID:
        *(uint8_t *)args = lsm6dso->id;
        break;
    default:
        return OS_ERROR;
    }
    return result;
}

static struct os_sensor_ops lsm6dso_gyro_ops =
{
    lsm6dso_gyro_sensor_fetch_data,
    lsm6dso_gyro_sensor_control
};

static int os_hw_lsm6dso_gyro_init(void)
{
    os_int8_t       result;
    lsm6dso_info_t *lsm6dso = NULL;

    lsm6dso = lsm6dso_sensor_init(OS_LSM6DSO_I2C_BUS_NAME, OS_LSM6DSO_I2C_ADDR, 0);
    if (lsm6dso == NULL)
    {
        LOG_E(DBG_TAG,"lsm6dso init failed.");
        goto __exit;
    }

    lsm6dso->sensor.info.type       = OS_SENSOR_CLASS_GYRO;
    lsm6dso->sensor.info.vendor     = OS_SENSOR_VENDOR_INVENSENSE;
    lsm6dso->sensor.info.model      = "lsm6dso";
    lsm6dso->sensor.info.unit       = OS_SENSOR_UNIT_MDPS;
    lsm6dso->sensor.info.intf_type  = OS_SENSOR_INTF_I2C;
    lsm6dso->sensor.info.range_max  = 16000;
    lsm6dso->sensor.info.range_min  = -16000;
    lsm6dso->sensor.info.period_min = 5;
    lsm6dso->sensor.ops             = &lsm6dso_gyro_ops;

    result = os_hw_sensor_register(&lsm6dso->sensor, "lsm6dso", OS_NULL);
    if (result != OS_EOK)
    {
        LOG_E(DBG_TAG,"device register err code: %d", result);
        goto __exit;
    }

    LOG_I(DBG_TAG,"lsm6dso init success");
    return OS_EOK;

__exit:
    if (lsm6dso)
    {
        os_free(lsm6dso);
    }
    return OS_ERROR;
}

OS_DEVICE_INIT(os_hw_lsm6dso_gyro_init,OS_INIT_SUBLEVEL_LOW);










