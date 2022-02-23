/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with 
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * @file        stmpe811.c
 *
 * @brief       stmpe811
 *
 * @details     stmpe811
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <device.h>
#include <os_clock.h>
#include <i2c.h>
#include <string.h>
#include <stdlib.h>
#include <os_errno.h>
#include <os_memory.h>
#include <pin/pin.h>
#include <string.h>
#include <drv_gpio.h>
#define LOG_TAG "gt9147"
#define DBG_LVL DBG_INFO
#include <drv_log.h>

#include "touch.h"

/* Chip IDs */
#define STMPE811_ID 0x0811

/* Identification registers & System Control */
#define STMPE811_REG_CHP_ID_LSB 0x00
#define STMPE811_REG_CHP_ID_MSB 0x01
#define STMPE811_REG_ID_VER     0x02

/* Global interrupt Enable bit */
#define STMPE811_GIT_EN 0x01

/* IO expander functionalities */
#define STMPE811_ADC_FCT      0x01
#define STMPE811_TS_FCT       0x02
#define STMPE811_IO_FCT       0x04
#define STMPE811_TEMPSENS_FCT 0x08

/* Global Interrupts definitions */ 
#define STMPE811_GIT_IO                 0x80  /* IO interrupt                   */
#define STMPE811_GIT_ADC                0x40  /* ADC interrupt                  */
#define STMPE811_GIT_TEMP               0x20  /* Not implemented                */
#define STMPE811_GIT_FE                 0x10  /* FIFO empty interrupt           */
#define STMPE811_GIT_FF                 0x08  /* FIFO full interrupt            */
#define STMPE811_GIT_FOV                0x04  /* FIFO overflowed interrupt      */
#define STMPE811_GIT_FTH                0x02  /* FIFO above threshold interrupt */
#define STMPE811_GIT_TOUCH              0x01  /* Touch is detected interrupt    */
#define STMPE811_ALL_GIT                0x1F  /* All global interrupts          */
#define STMPE811_TS_IT                  (STMPE811_GIT_TOUCH | STMPE811_GIT_FTH |  STMPE811_GIT_FOV | STMPE811_GIT_FF | STMPE811_GIT_FE) /* Touch screen interrupts */
    
/* General Control Registers */ 
#define STMPE811_REG_SYS_CTRL1          0x03
#define STMPE811_REG_SYS_CTRL2          0x04
#define STMPE811_REG_SPI_CFG            0x08 

/* Interrupt system Registers */
#define STMPE811_REG_INT_CTRL   0x09
#define STMPE811_REG_INT_EN     0x0A
#define STMPE811_REG_INT_STA    0x0B
#define STMPE811_REG_IO_INT_EN  0x0C
#define STMPE811_REG_IO_INT_STA 0x0D

/* IO Registers */
#define STMPE811_REG_IO_SET_PIN 0x10
#define STMPE811_REG_IO_CLR_PIN 0x11
#define STMPE811_REG_IO_MP_STA  0x12
#define STMPE811_REG_IO_DIR     0x13
#define STMPE811_REG_IO_ED      0x14
#define STMPE811_REG_IO_RE      0x15
#define STMPE811_REG_IO_FE      0x16
#define STMPE811_REG_IO_AF      0x17

/* ADC Registers */
#define STMPE811_REG_ADC_INT_EN   0x0E
#define STMPE811_REG_ADC_INT_STA  0x0F
#define STMPE811_REG_ADC_CTRL1    0x20
#define STMPE811_REG_ADC_CTRL2    0x21
#define STMPE811_REG_ADC_CAPT     0x22
#define STMPE811_REG_ADC_DATA_CH0 0x30
#define STMPE811_REG_ADC_DATA_CH1 0x32
#define STMPE811_REG_ADC_DATA_CH2 0x34
#define STMPE811_REG_ADC_DATA_CH3 0x36
#define STMPE811_REG_ADC_DATA_CH4 0x38
#define STMPE811_REG_ADC_DATA_CH5 0x3A
#define STMPE811_REG_ADC_DATA_CH6 0x3B
#define STMPE811_REG_ADC_DATA_CH7 0x3C

/* Touch Screen Registers */
#define STMPE811_REG_TSC_CTRL         0x40
#define STMPE811_REG_TSC_CFG          0x41
#define STMPE811_REG_WDM_TR_X         0x42
#define STMPE811_REG_WDM_TR_Y         0x44
#define STMPE811_REG_WDM_BL_X         0x46
#define STMPE811_REG_WDM_BL_Y         0x48
#define STMPE811_REG_FIFO_TH          0x4A
#define STMPE811_REG_FIFO_STA         0x4B
#define STMPE811_REG_FIFO_SIZE        0x4C
#define STMPE811_REG_TSC_DATA_X       0x4D
#define STMPE811_REG_TSC_DATA_Y       0x4F
#define STMPE811_REG_TSC_DATA_Z       0x51
#define STMPE811_REG_TSC_DATA_XYZ     0x52
#define STMPE811_REG_TSC_FRACT_XYZ    0x56
#define STMPE811_REG_TSC_DATA_INC     0x57
#define STMPE811_REG_TSC_DATA_NON_INC 0xD7
#define STMPE811_REG_TSC_I_DRIVE      0x58
#define STMPE811_REG_TSC_SHIELD       0x59

/* Touch Screen Pins definition */
#define STMPE811_TOUCH_YD     STMPE811_PIN_7
#define STMPE811_TOUCH_XD     STMPE811_PIN_6
#define STMPE811_TOUCH_YU     STMPE811_PIN_5
#define STMPE811_TOUCH_XU     STMPE811_PIN_4
#define STMPE811_TOUCH_IO_ALL (uint32_t)(STMPE811_TOUCH_YD | STMPE811_TOUCH_XD | STMPE811_TOUCH_YU | STMPE811_TOUCH_XU)

/* IO Pins definition */
#define STMPE811_PIN_0   0x01
#define STMPE811_PIN_1   0x02
#define STMPE811_PIN_2   0x04
#define STMPE811_PIN_3   0x08
#define STMPE811_PIN_4   0x10
#define STMPE811_PIN_5   0x20
#define STMPE811_PIN_6   0x40
#define STMPE811_PIN_7   0x80
#define STMPE811_PIN_ALL 0xFF

/* IO Pins directions */
#define STMPE811_DIRECTION_IN  0x00
#define STMPE811_DIRECTION_OUT 0x01

/* IO IT types */
#define STMPE811_TYPE_LEVEL 0x00
#define STMPE811_TYPE_EDGE  0x02

/* IO IT polarity */
#define STMPE811_POLARITY_LOW  0x00
#define STMPE811_POLARITY_HIGH 0x04

/* IO Pin IT edge modes */
#define STMPE811_EDGE_FALLING 0x01
#define STMPE811_EDGE_RISING  0x02

/* TS registers masks */
#define STMPE811_TS_CTRL_ENABLE 0x01
#define STMPE811_TS_CTRL_STATUS 0x80

typedef enum
{
    IO_MODE_INPUT = 0,       /* input floating */
    IO_MODE_OUTPUT,          /* output Push Pull */
    IO_MODE_IT_RISING_EDGE,  /* float input - irq detect on rising edge */
    IO_MODE_IT_FALLING_EDGE, /* float input - irq detect on falling edge */
    IO_MODE_IT_LOW_LEVEL,    /* float input - irq detect on low level */
    IO_MODE_IT_HIGH_LEVEL,   /* float input - irq detect on high level */
    /* following modes only available on MFX*/
    IO_MODE_ANALOG,             /* analog mode */
    IO_MODE_OFF,                /* when pin isn't used*/
    IO_MODE_INPUT_PU,           /* input with internal pull up resistor */
    IO_MODE_INPUT_PD,           /* input with internal pull down resistor */
    IO_MODE_OUTPUT_OD,          /* Open Drain output without internal resistor */
    IO_MODE_OUTPUT_OD_PU,       /* Open Drain output with  internal pullup resistor */
    IO_MODE_OUTPUT_OD_PD,       /* Open Drain output with  internal pulldown resistor */
    IO_MODE_OUTPUT_PP,          /* PushPull output without internal resistor */
    IO_MODE_OUTPUT_PP_PU,       /* PushPull output with  internal pullup resistor */
    IO_MODE_OUTPUT_PP_PD,       /* PushPull output with  internal pulldown resistor */
    IO_MODE_IT_RISING_EDGE_PU,  /* push up resistor input - irq on rising edge  */
    IO_MODE_IT_RISING_EDGE_PD,  /* push dw resistor input - irq on rising edge  */
    IO_MODE_IT_FALLING_EDGE_PU, /* push up resistor input - irq on falling edge */
    IO_MODE_IT_FALLING_EDGE_PD, /* push dw resistor input - irq on falling edge */
    IO_MODE_IT_LOW_LEVEL_PU,    /* push up resistor input - irq detect on low level */
    IO_MODE_IT_LOW_LEVEL_PD,    /* push dw resistor input - irq detect on low level */
    IO_MODE_IT_HIGH_LEVEL_PU,   /* push up resistor input - irq detect on high level */
    IO_MODE_IT_HIGH_LEVEL_PD,   /* push dw resistor input - irq detect on high level */

} IO_ModeTypedef;

struct stmpe811_touch {
    os_touch_t touch_device;

    struct os_i2c_bus_device *i2c_bus;
    os_uint16_t               i2c_addr;

    uint16_t x_range;
    uint16_t y_range;
};

void     stmpe811_Reset(struct stmpe811_touch *stmpe811);
uint16_t stmpe811_ReadID(struct stmpe811_touch *stmpe811);
void     stmpe811_EnableGlobalIT(struct stmpe811_touch *stmpe811);
void     stmpe811_DisableGlobalIT(struct stmpe811_touch *stmpe811);
void     stmpe811_EnableITSource(struct stmpe811_touch *stmpe811, uint8_t Source);
void     stmpe811_DisableITSource(struct stmpe811_touch *stmpe811, uint8_t Source);
void     stmpe811_SetITPolarity(struct stmpe811_touch *stmpe811, uint8_t Polarity);
void     stmpe811_SetITType(struct stmpe811_touch *stmpe811, uint8_t Type);
uint8_t  stmpe811_GlobalITStatus(struct stmpe811_touch *stmpe811, uint8_t Source);
uint8_t  stmpe811_ReadGITStatus(struct stmpe811_touch *stmpe811, uint8_t Source);
void     stmpe811_ClearGlobalIT(struct stmpe811_touch *stmpe811, uint8_t Source);

void     stmpe811_IO_Start(struct stmpe811_touch *stmpe811, uint32_t IO_Pin);
uint8_t  stmpe811_IO_Config(struct stmpe811_touch *stmpe811, uint32_t IO_Pin, IO_ModeTypedef IO_Mode);
void     stmpe811_IO_InitPin(struct stmpe811_touch *stmpe811, uint32_t IO_Pin, uint8_t Direction);
void     stmpe811_IO_EnableAF(struct stmpe811_touch *stmpe811, uint32_t IO_Pin);
void     stmpe811_IO_DisableAF(struct stmpe811_touch *stmpe811, uint32_t IO_Pin);
void     stmpe811_IO_SetEdgeMode(struct stmpe811_touch *stmpe811, uint32_t IO_Pin, uint8_t Edge);
void     stmpe811_IO_WritePin(struct stmpe811_touch *stmpe811, uint32_t IO_Pin, uint8_t PinState);
uint32_t stmpe811_IO_ReadPin(struct stmpe811_touch *stmpe811, uint32_t IO_Pin);
void     stmpe811_IO_EnableIT(struct stmpe811_touch *stmpe811);
void     stmpe811_IO_DisableIT(struct stmpe811_touch *stmpe811);
void     stmpe811_IO_EnablePinIT(struct stmpe811_touch *stmpe811, uint32_t IO_Pin);
void     stmpe811_IO_DisablePinIT(struct stmpe811_touch *stmpe811, uint32_t IO_Pin);
uint32_t stmpe811_IO_ITStatus(struct stmpe811_touch *stmpe811, uint32_t IO_Pin);
void     stmpe811_IO_ClearIT(struct stmpe811_touch *stmpe811, uint32_t IO_Pin);

void    stmpe811_TS_Start(struct stmpe811_touch *stmpe811);
uint8_t stmpe811_TS_DetectTouch(struct stmpe811_touch *stmpe811);
void    stmpe811_TS_GetXY(struct stmpe811_touch *stmpe811, uint16_t *X, uint16_t *Y);
void    stmpe811_TS_EnableIT(struct stmpe811_touch *stmpe811);
void    stmpe811_TS_DisableIT(struct stmpe811_touch *stmpe811);
uint8_t stmpe811_TS_ITStatus(struct stmpe811_touch *stmpe811);
void    stmpe811_TS_ClearIT(struct stmpe811_touch *stmpe811);

static void IOE_Write(struct stmpe811_touch *stmpe811, uint8_t reg, uint8_t value)
{
    unsigned char buff[2] = {reg, value};
    os_i2c_master_send(stmpe811->i2c_bus, stmpe811->i2c_addr, 0, buff, 2);
}

static uint8_t IOE_Read(struct stmpe811_touch *stmpe811, uint8_t reg)
{
    unsigned char value;
    os_i2c_master_send(stmpe811->i2c_bus, stmpe811->i2c_addr, 0, &reg, 1);
    os_i2c_master_recv(stmpe811->i2c_bus, stmpe811->i2c_addr, 0, &value, 1);
    return value;
}

uint16_t IOE_ReadMultiple(struct stmpe811_touch *stmpe811, uint8_t reg, uint8_t *buffer, uint16_t length)
{
    os_i2c_master_send(stmpe811->i2c_bus, stmpe811->i2c_addr, 0, &reg, 1);
    os_i2c_master_recv(stmpe811->i2c_bus, stmpe811->i2c_addr, 0, buffer, length);
    return 0;
}

static void IOE_Delay(uint32_t delay)
{
    os_task_msleep(delay);
}

void stmpe811_Reset(struct stmpe811_touch *stmpe811)
{
    /* Power Down the stmpe811 */
    IOE_Write(stmpe811, STMPE811_REG_SYS_CTRL1, 2);

    /* Wait for a delay to ensure registers erasing */
    IOE_Delay(10);

    /* Power On the Codec after the power off => all registers are reinitialized */
    IOE_Write(stmpe811, STMPE811_REG_SYS_CTRL1, 0);

    /* Wait for a delay to ensure registers erasing */
    IOE_Delay(2);
}

uint16_t stmpe811_ReadID(struct stmpe811_touch *stmpe811)
{
    /* Return the device ID value */
    return ((IOE_Read(stmpe811, STMPE811_REG_CHP_ID_LSB) << 8) | (IOE_Read(stmpe811, STMPE811_REG_CHP_ID_MSB)));
}

void stmpe811_EnableGlobalIT(struct stmpe811_touch *stmpe811)
{
    uint8_t tmp = 0;

    /* Read the Interrupt Control register  */
    tmp = IOE_Read(stmpe811, STMPE811_REG_INT_CTRL);

    /* Set the global interrupts to be Enabled */
    tmp |= (uint8_t)STMPE811_GIT_EN;

    /* Write Back the Interrupt Control register */
    IOE_Write(stmpe811, STMPE811_REG_INT_CTRL, tmp);
}

void stmpe811_DisableGlobalIT(struct stmpe811_touch *stmpe811)
{
    uint8_t tmp = 0;

    /* Read the Interrupt Control register  */
    tmp = IOE_Read(stmpe811, STMPE811_REG_INT_CTRL);

    /* Set the global interrupts to be Disabled */
    tmp &= ~(uint8_t)STMPE811_GIT_EN;

    /* Write Back the Interrupt Control register */
    IOE_Write(stmpe811, STMPE811_REG_INT_CTRL, tmp);
}

void stmpe811_EnableITSource(struct stmpe811_touch *stmpe811, uint8_t Source)
{
    uint8_t tmp = 0;

    /* Get the current value of the INT_EN register */
    tmp = IOE_Read(stmpe811, STMPE811_REG_INT_EN);

    /* Set the interrupts to be Enabled */
    tmp |= Source;

    /* Set the register */
    IOE_Write(stmpe811, STMPE811_REG_INT_EN, tmp);
}

void stmpe811_DisableITSource(struct stmpe811_touch *stmpe811, uint8_t Source)
{
    uint8_t tmp = 0;

    /* Get the current value of the INT_EN register */
    tmp = IOE_Read(stmpe811, STMPE811_REG_INT_EN);

    /* Set the interrupts to be Enabled */
    tmp &= ~Source;

    /* Set the register */
    IOE_Write(stmpe811, STMPE811_REG_INT_EN, tmp);
}

void stmpe811_SetITPolarity(struct stmpe811_touch *stmpe811, uint8_t Polarity)
{
    uint8_t tmp = 0;

    /* Get the current register value */
    tmp = IOE_Read(stmpe811, STMPE811_REG_INT_CTRL);

    /* Mask the polarity bits */
    tmp &= ~(uint8_t)0x04;

    /* Modify the Interrupt Output line configuration */
    tmp |= Polarity;

    /* Set the new register value */
    IOE_Write(stmpe811, STMPE811_REG_INT_CTRL, tmp);
}

void stmpe811_SetITType(struct stmpe811_touch *stmpe811, uint8_t Type)
{
    uint8_t tmp = 0;

    /* Get the current register value */
    tmp = IOE_Read(stmpe811, STMPE811_REG_INT_CTRL);

    /* Mask the type bits */
    tmp &= ~(uint8_t)0x02;

    /* Modify the Interrupt Output line configuration */
    tmp |= Type;

    /* Set the new register value */
    IOE_Write(stmpe811, STMPE811_REG_INT_CTRL, tmp);
}

uint8_t stmpe811_GlobalITStatus(struct stmpe811_touch *stmpe811, uint8_t Source)
{
    /* Return the global IT source status */
    return ((IOE_Read(stmpe811, STMPE811_REG_INT_STA) & Source) == Source);
}

uint8_t stmpe811_ReadGITStatus(struct stmpe811_touch *stmpe811, uint8_t Source)
{
    /* Return the global IT source status */
    return ((IOE_Read(stmpe811, STMPE811_REG_INT_STA) & Source));
}

void stmpe811_ClearGlobalIT(struct stmpe811_touch *stmpe811, uint8_t Source)
{
    /* Write 1 to the bits that have to be cleared */
    IOE_Write(stmpe811, STMPE811_REG_INT_STA, Source);
}

void stmpe811_IO_Start(struct stmpe811_touch *stmpe811, uint32_t IO_Pin)
{
    uint8_t mode;

    /* Get the current register value */
    mode = IOE_Read(stmpe811, STMPE811_REG_SYS_CTRL2);

    /* Set the Functionalities to be Disabled */
    mode &= ~(STMPE811_IO_FCT | STMPE811_ADC_FCT);

    /* Write the new register value */
    IOE_Write(stmpe811, STMPE811_REG_SYS_CTRL2, mode);

    /* Disable AF for the selected IO pin(s) */
    stmpe811_IO_DisableAF(stmpe811, (uint8_t)IO_Pin);
}

uint8_t stmpe811_IO_Config(struct stmpe811_touch *stmpe811, uint32_t IO_Pin, IO_ModeTypedef IO_Mode)
{
    uint8_t error_code = 0;

    /* Configure IO pin according to selected IO mode */
    switch (IO_Mode)
    {
    case IO_MODE_INPUT: /* Input mode */
        stmpe811_IO_InitPin(stmpe811, IO_Pin, STMPE811_DIRECTION_IN);
        break;

    case IO_MODE_OUTPUT: /* Output mode */
        stmpe811_IO_InitPin(stmpe811, IO_Pin, STMPE811_DIRECTION_OUT);
        break;

    case IO_MODE_IT_RISING_EDGE: /* Interrupt rising edge mode */
        stmpe811_IO_EnableIT(stmpe811);
        stmpe811_IO_EnablePinIT(stmpe811, IO_Pin);
        stmpe811_IO_InitPin(stmpe811, IO_Pin, STMPE811_DIRECTION_IN);
        stmpe811_SetITType(stmpe811, STMPE811_TYPE_EDGE);
        stmpe811_IO_SetEdgeMode(stmpe811, IO_Pin, STMPE811_EDGE_RISING);
        break;

    case IO_MODE_IT_FALLING_EDGE: /* Interrupt falling edge mode */
        stmpe811_IO_EnableIT(stmpe811);
        stmpe811_IO_EnablePinIT(stmpe811, IO_Pin);
        stmpe811_IO_InitPin(stmpe811, IO_Pin, STMPE811_DIRECTION_IN);
        stmpe811_SetITType(stmpe811, STMPE811_TYPE_EDGE);
        stmpe811_IO_SetEdgeMode(stmpe811, IO_Pin, STMPE811_EDGE_FALLING);
        break;

    case IO_MODE_IT_LOW_LEVEL: /* Low level interrupt mode */
        stmpe811_IO_EnableIT(stmpe811);
        stmpe811_IO_EnablePinIT(stmpe811, IO_Pin);
        stmpe811_IO_InitPin(stmpe811, IO_Pin, STMPE811_DIRECTION_IN);
        stmpe811_SetITType(stmpe811, STMPE811_TYPE_LEVEL);
        stmpe811_SetITPolarity(stmpe811, STMPE811_POLARITY_LOW);
        break;

    case IO_MODE_IT_HIGH_LEVEL: /* High level interrupt mode */
        stmpe811_IO_EnableIT(stmpe811);
        stmpe811_IO_EnablePinIT(stmpe811, IO_Pin);
        stmpe811_IO_InitPin(stmpe811, IO_Pin, STMPE811_DIRECTION_IN);
        stmpe811_SetITType(stmpe811, STMPE811_TYPE_LEVEL);
        stmpe811_SetITPolarity(stmpe811, STMPE811_POLARITY_HIGH);
        break;

    default:
        error_code = (uint8_t)IO_Mode;
        break;
    }
    return error_code;
}

void stmpe811_IO_InitPin(struct stmpe811_touch *stmpe811, uint32_t IO_Pin, uint8_t Direction)
{
    uint8_t tmp = 0;

    /* Get all the Pins direction */
    tmp = IOE_Read(stmpe811, STMPE811_REG_IO_DIR);

    /* Set the selected pin direction */
    if (Direction != STMPE811_DIRECTION_IN)
    {
        tmp |= (uint8_t)IO_Pin;
    }
    else
    {
        tmp &= ~(uint8_t)IO_Pin;
    }

    /* Write the register new value */
    IOE_Write(stmpe811, STMPE811_REG_IO_DIR, tmp);
}

void stmpe811_IO_DisableAF(struct stmpe811_touch *stmpe811, uint32_t IO_Pin)
{
    uint8_t tmp = 0;

    /* Get the current state of the IO_AF register */
    tmp = IOE_Read(stmpe811, STMPE811_REG_IO_AF);

    /* Enable the selected pins alternate function */
    tmp |= (uint8_t)IO_Pin;

    /* Write back the new value in IO AF register */
    IOE_Write(stmpe811, STMPE811_REG_IO_AF, tmp);
}

void stmpe811_IO_EnableAF(struct stmpe811_touch *stmpe811, uint32_t IO_Pin)
{
    uint8_t tmp = 0;

    /* Get the current register value */
    tmp = IOE_Read(stmpe811, STMPE811_REG_IO_AF);

    /* Enable the selected pins alternate function */
    tmp &= ~(uint8_t)IO_Pin;

    /* Write back the new register value */
    IOE_Write(stmpe811, STMPE811_REG_IO_AF, tmp);
}

void stmpe811_IO_SetEdgeMode(struct stmpe811_touch *stmpe811, uint32_t IO_Pin, uint8_t Edge)
{
    uint8_t tmp1 = 0, tmp2 = 0;

    /* Get the current registers values */
    tmp1 = IOE_Read(stmpe811, STMPE811_REG_IO_FE);
    tmp2 = IOE_Read(stmpe811, STMPE811_REG_IO_RE);

    /* Disable the Falling Edge */
    tmp1 &= ~(uint8_t)IO_Pin;

    /* Disable the Falling Edge */
    tmp2 &= ~(uint8_t)IO_Pin;

    /* Enable the Falling edge if selected */
    if (Edge & STMPE811_EDGE_FALLING)
    {
        tmp1 |= (uint8_t)IO_Pin;
    }

    /* Enable the Rising edge if selected */
    if (Edge & STMPE811_EDGE_RISING)
    {
        tmp2 |= (uint8_t)IO_Pin;
    }

    /* Write back the new registers values */
    IOE_Write(stmpe811, STMPE811_REG_IO_FE, tmp1);
    IOE_Write(stmpe811, STMPE811_REG_IO_RE, tmp2);
}

void stmpe811_IO_WritePin(struct stmpe811_touch *stmpe811, uint32_t IO_Pin, uint8_t PinState)
{
    /* Apply the bit value to the selected pin */
    if (PinState != 0)
    {
        /* Set the register */
        IOE_Write(stmpe811, STMPE811_REG_IO_SET_PIN, (uint8_t)IO_Pin);
    }
    else
    {
        /* Set the register */
        IOE_Write(stmpe811, STMPE811_REG_IO_CLR_PIN, (uint8_t)IO_Pin);
    }
}

uint32_t stmpe811_IO_ReadPin(struct stmpe811_touch *stmpe811, uint32_t IO_Pin)
{
    return ((uint32_t)(IOE_Read(stmpe811, STMPE811_REG_IO_MP_STA) & (uint8_t)IO_Pin));
}

void stmpe811_IO_EnableIT(struct stmpe811_touch *stmpe811)
{
    /* Enable global IO IT source */
    stmpe811_EnableITSource(stmpe811, STMPE811_GIT_IO);

    /* Enable global interrupt */
    stmpe811_EnableGlobalIT(stmpe811);
}

void stmpe811_IO_DisableIT(struct stmpe811_touch *stmpe811)
{
    /* Disable the global interrupt */
    stmpe811_DisableGlobalIT(stmpe811);

    /* Disable global IO IT source */
    stmpe811_DisableITSource(stmpe811, STMPE811_GIT_IO);
}

void stmpe811_IO_EnablePinIT(struct stmpe811_touch *stmpe811, uint32_t IO_Pin)
{
    uint8_t tmp = 0;

    /* Get the IO interrupt state */
    tmp = IOE_Read(stmpe811, STMPE811_REG_IO_INT_EN);

    /* Set the interrupts to be enabled */
    tmp |= (uint8_t)IO_Pin;

    /* Write the register new value */
    IOE_Write(stmpe811, STMPE811_REG_IO_INT_EN, tmp);
}

void stmpe811_IO_DisablePinIT(struct stmpe811_touch *stmpe811, uint32_t IO_Pin)
{
    uint8_t tmp = 0;

    /* Get the IO interrupt state */
    tmp = IOE_Read(stmpe811, STMPE811_REG_IO_INT_EN);

    /* Set the interrupts to be Disabled */
    tmp &= ~(uint8_t)IO_Pin;

    /* Write the register new value */
    IOE_Write(stmpe811, STMPE811_REG_IO_INT_EN, tmp);
}

uint32_t stmpe811_IO_ITStatus(struct stmpe811_touch *stmpe811, uint32_t IO_Pin)
{
    /* Get the Interrupt status */
    return (IOE_Read(stmpe811, STMPE811_REG_IO_INT_STA) & (uint8_t)IO_Pin);
}

void stmpe811_IO_ClearIT(struct stmpe811_touch *stmpe811, uint32_t IO_Pin)
{
    /* Clear the global IO IT pending bit */
    stmpe811_ClearGlobalIT(stmpe811, STMPE811_GIT_IO);

    /* Clear the IO IT pending bit(s) */
    IOE_Write(stmpe811, STMPE811_REG_IO_INT_STA, (uint8_t)IO_Pin);

    /* Clear the Edge detection pending bit*/
    IOE_Write(stmpe811, STMPE811_REG_IO_ED, (uint8_t)IO_Pin);

    /* Clear the Rising edge pending bit */
    IOE_Write(stmpe811, STMPE811_REG_IO_RE, (uint8_t)IO_Pin);

    /* Clear the Falling edge pending bit */
    IOE_Write(stmpe811, STMPE811_REG_IO_FE, (uint8_t)IO_Pin);
}

void stmpe811_TS_Start(struct stmpe811_touch *stmpe811)
{
    uint8_t mode;

    /* Get the current register value */
    mode = IOE_Read(stmpe811, STMPE811_REG_SYS_CTRL2);

    /* Set the Functionalities to be Enabled */
    mode &= ~(STMPE811_IO_FCT);

    /* Write the new register value */
    IOE_Write(stmpe811, STMPE811_REG_SYS_CTRL2, mode);

    /* Select TSC pins in TSC alternate mode */
    stmpe811_IO_EnableAF(stmpe811, STMPE811_TOUCH_IO_ALL);

    /* Set the Functionalities to be Enabled */
    mode &= ~(STMPE811_TS_FCT | STMPE811_ADC_FCT);

    /* Set the new register value */
    IOE_Write(stmpe811, STMPE811_REG_SYS_CTRL2, mode);

    /* Select Sample Time, bit number and ADC Reference */
    IOE_Write(stmpe811, STMPE811_REG_ADC_CTRL1, 0x49);

    /* Wait for 2 ms */
    IOE_Delay(2);

    /* Select the ADC clock speed: 3.25 MHz */
    IOE_Write(stmpe811, STMPE811_REG_ADC_CTRL2, 0x01);

    /* Select 2 nF filter capacitor */
    /* Configuration:
       - Touch average control    : 4 samples
       - Touch delay time         : 500 uS
       - Panel driver setting time: 500 uS
    */
    IOE_Write(stmpe811, STMPE811_REG_TSC_CFG, 0x9A);

    /* Configure the Touch FIFO threshold: single point reading */
    IOE_Write(stmpe811, STMPE811_REG_FIFO_TH, 0x01);

    /* Clear the FIFO memory content. */
    IOE_Write(stmpe811, STMPE811_REG_FIFO_STA, 0x01);

    /* Put the FIFO back into operation mode  */
    IOE_Write(stmpe811, STMPE811_REG_FIFO_STA, 0x00);

    /* Set the range and accuracy pf the pressure measurement (Z) :
       - Fractional part :7
       - Whole part      :1
    */
    IOE_Write(stmpe811, STMPE811_REG_TSC_FRACT_XYZ, 0x01);

    /* Set the driving capability (limit) of the device for TSC pins: 50mA */
    IOE_Write(stmpe811, STMPE811_REG_TSC_I_DRIVE, 0x01);

    /* Touch screen control configuration (enable TSC):
       - No window tracking index
       - XYZ acquisition mode
     */
    IOE_Write(stmpe811, STMPE811_REG_TSC_CTRL, 0x01);

    /*  Clear all the status pending bits if any */
    IOE_Write(stmpe811, STMPE811_REG_INT_STA, 0xFF);

    /* Wait for 2 ms delay */
    IOE_Delay(2);
}

uint8_t stmpe811_TS_DetectTouch(struct stmpe811_touch *stmpe811)
{
    uint8_t state;
    uint8_t ret = 0;

    state = ((IOE_Read(stmpe811, STMPE811_REG_TSC_CTRL) & (uint8_t)STMPE811_TS_CTRL_STATUS) == (uint8_t)0x80);

    if (state > 0)
    {
        if (IOE_Read(stmpe811, STMPE811_REG_FIFO_SIZE) > 0)
        {
            ret = 1;
        }
    }
    else
    {
        /* Reset FIFO */
        IOE_Write(stmpe811, STMPE811_REG_FIFO_STA, 0x01);
        /* Enable the FIFO again */
        IOE_Write(stmpe811, STMPE811_REG_FIFO_STA, 0x00);
    }

    return ret;
}

void stmpe811_TS_GetXY(struct stmpe811_touch *stmpe811, uint16_t *X, uint16_t *Y)
{
    uint8_t  dataXYZ[4];
    uint32_t uldataXYZ;

    IOE_ReadMultiple(stmpe811, STMPE811_REG_TSC_DATA_NON_INC, dataXYZ, sizeof(dataXYZ));

    /* Calculate positions values */
    uldataXYZ = (dataXYZ[0] << 24) | (dataXYZ[1] << 16) | (dataXYZ[2] << 8) | (dataXYZ[3] << 0);
    *X        = (uldataXYZ >> 20) & 0x00000FFF;
    *Y        = (uldataXYZ >> 8) & 0x00000FFF;

    /* Reset FIFO */
    IOE_Write(stmpe811, STMPE811_REG_FIFO_STA, 0x01);
    /* Enable the FIFO again */
    IOE_Write(stmpe811, STMPE811_REG_FIFO_STA, 0x00);
}

void stmpe811_TS_EnableIT(struct stmpe811_touch *stmpe811)
{
    /* Enable global TS IT source */
    stmpe811_EnableITSource(stmpe811, STMPE811_TS_IT);

    /* Enable global interrupt */
    stmpe811_EnableGlobalIT(stmpe811);
}

void stmpe811_TS_DisableIT(struct stmpe811_touch *stmpe811)
{
    /* Disable global interrupt */
    stmpe811_DisableGlobalIT(stmpe811);

    /* Disable global TS IT source */
    stmpe811_DisableITSource(stmpe811, STMPE811_TS_IT);
}

uint8_t stmpe811_TS_ITStatus(struct stmpe811_touch *stmpe811)
{
    /* Return TS interrupts status */
    return (stmpe811_ReadGITStatus(stmpe811, STMPE811_TS_IT));
}

void stmpe811_TS_ClearIT(struct stmpe811_touch *stmpe811)
{
    /* Clear the global TS IT source */
    stmpe811_ClearGlobalIT(stmpe811, STMPE811_TS_IT);
}

static os_size_t stmpe811_read_point(struct os_touch_device *touch, struct os_touch_data *data, os_size_t read_num)
{
    uint16_t input_x  = 0;
    uint16_t input_y  = 0;
    uint16_t input_w  = 0;
    uint16_t new_data = 0;

    struct stmpe811_touch *stmpe811 = (struct stmpe811_touch *)touch;

    new_data = stmpe811_TS_DetectTouch(stmpe811);
    if (new_data == 0)
        return 0;

    stmpe811_TS_GetXY(stmpe811, &input_x, &input_y);

    data->event        = OS_TOUCH_EVENT_DOWN;
    data->timestamp    = os_touch_get_ts();
    data->width        = input_w;
    data->x_coordinate = input_x;
    data->y_coordinate = input_y;
    data->track_id     = 0;

    return 1;
}

static os_err_t stmpe811_control(struct os_touch_device *device, int cmd, void *data)
{
    struct stmpe811_touch *stmpe811 = (struct stmpe811_touch *)device;

    switch (cmd)
    {
    case OS_TOUCH_CTRL_GET_ID:
    {
        uint16_t id  = stmpe811_ReadID(stmpe811);
        uint8_t *idp = data;
        *idp++       = id & 0xff;
        *idp++       = id >> 8;
        break;
    }

    case OS_TOUCH_CTRL_GET_INFO:
    {
        struct os_touch_info *info = data;
        *info                      = device->info;
        break;
    }

    case OS_TOUCH_CTRL_SET_X_RANGE: /* set x range */
    {
        device->info.range_x = *(os_int32_t *)data;
        break;
    }
    case OS_TOUCH_CTRL_SET_Y_RANGE: /* set y range */
    {
        device->info.range_y = *(os_int32_t *)data;
        break;
    }
    case OS_TOUCH_CTRL_SET_X_TO_Y: /* change x y */
    {
        break;
    }
    case OS_TOUCH_CTRL_SET_MODE: /* change int trig type */
    {
        break;
    }

    default:
    {
        break;
    }
    }

    return OS_EOK;
}

static struct os_touch_ops stmpe811_touch_ops =
{
    .touch_readpoint = stmpe811_read_point,
    .touch_control   = stmpe811_control,
};

static int os_hw_stmpe811_init(void)
{
    struct stmpe811_touch *stmpe811 = os_calloc(1, sizeof(struct stmpe811_touch));
    OS_ASSERT(stmpe811);

    stmpe811->x_range  = 4000;
    stmpe811->y_range  = 4000;
    stmpe811->i2c_addr = OS_STMPE811_I2C_ADDR;
    stmpe811->i2c_bus  = os_i2c_bus_device_find(OS_STMPE811_I2C_BUS_NAME);
    OS_ASSERT(stmpe811->i2c_bus);

    /* Generate stmpe811 Software reset */
    stmpe811_Reset(stmpe811);
    stmpe811_TS_Start(stmpe811);

    os_touch_t *touch_device = &stmpe811->touch_device;

    /* register touch device */
    touch_device->info.type      = OS_TOUCH_TYPE_CAPACITANCE;
    touch_device->info.vendor    = OS_TOUCH_VENDOR_UNKNOWN;
    touch_device->info.point_num = 1;
    touch_device->info.range_x   = stmpe811->x_range;
    touch_device->info.range_y   = stmpe811->y_range;
    touch_device->ops            = &stmpe811_touch_ops;

    os_hw_touch_register(touch_device, "touch", stmpe811);

    return 0;
}

OS_CMPOENT_INIT(os_hw_stmpe811_init, OS_INIT_SUBLEVEL_LOW);
