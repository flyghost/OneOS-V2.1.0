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
 * @file        drv_sdram.c
 *
 * @brief       This file implements SDRAM driver for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_clock.h>
#include <sdram_port.h>

#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)

void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram)
{
    static FMC_SDRAM_CommandTypeDef command, *Command = &command;

    __IO uint32_t tmpmrd      = 0;
    uint32_t      target_bank = 0;

#if SDRAM_TARGET_BANK == 1
    target_bank = FMC_SDRAM_CMD_TARGET_BANK1;
#else
    target_bank = FMC_SDRAM_CMD_TARGET_BANK2;
#endif

    /* Configure a clock configuration enable command */
    Command->CommandMode            = FMC_SDRAM_CMD_CLK_ENABLE;
    Command->CommandTarget          = target_bank;
    Command->AutoRefreshNumber      = 1;
    Command->ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(hsdram, Command, 0x1000);

    /* Insert 100 ms delay */
    /* interrupt is not enable, just to delay some time. */
#ifndef SDRAM_INIT_DELAY
#define SDRAM_INIT_DELAY 0xffffff
#endif
    for (tmpmrd = 0; tmpmrd < SDRAM_INIT_DELAY; tmpmrd++);

    /* Configure a PALL (precharge all) command */
    Command->CommandMode            = FMC_SDRAM_CMD_PALL;
    Command->CommandTarget          = target_bank;
    Command->AutoRefreshNumber      = 1;
    Command->ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(hsdram, Command, 0x1000);

    /* Configure a Auto-Refresh command */
    Command->CommandMode            = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
    Command->CommandTarget          = target_bank;
    Command->AutoRefreshNumber      = 8;
    Command->ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(hsdram, Command, 0x1000);

    /* Program the external memory mode register */
#if SDRAM_DATA_WIDTH == 8
    tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_1 |
#elif SDRAM_DATA_WIDTH == 16
    tmpmrd      = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_2 |
#else
    tmpmrd                        = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_4 |
#endif
             SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL |
#if SDRAM_CAS_LATENCY == 3
             SDRAM_MODEREG_CAS_LATENCY_3 |
#else
             SDRAM_MODEREG_CAS_LATENCY_2 |
#endif
             SDRAM_MODEREG_OPERATING_MODE_STANDARD | SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

    Command->CommandMode            = FMC_SDRAM_CMD_LOAD_MODE;
    Command->CommandTarget          = target_bank;
    Command->AutoRefreshNumber      = 1;
    Command->ModeRegisterDefinition = tmpmrd;

    /* Send the command */
    HAL_SDRAM_SendCommand(hsdram, Command, 0x1000);

    /* Set the device refresh counter */
    HAL_SDRAM_ProgramRefreshRate(hsdram, SDRAM_REFRESH_COUNT);
}

static void MPU_Config(void)
{
#ifdef MPU_REGION_BASE

    MPU_Region_InitTypeDef MPU_InitStruct;

    /* Disable the MPU */
    HAL_MPU_Disable();

    /* Configure the MPU attributes for SDRAM */
    MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
    MPU_InitStruct.BaseAddress      = MPU_REGION_BASE;
    MPU_InitStruct.Size             = MPU_REGION_SIZE;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.IsBufferable     = MPU_ACCESS_BUFFERABLE;
    MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
    MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.Number           = MPU_REGION_NUMBER2;
    MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL1;
    MPU_InitStruct.SubRegionDisable = 0x00;
    MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;

    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    /* Enable the MPU */
    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

#endif
}

static os_err_t sdram_test(void)
{
    int i = 0;

    /* write data */
    for (i = 0; i < SDRAM_SIZE; i += 4)
    {
        *(__IO uint32_t *)(SDRAM_BASE + i) = i;
    }

    /* read data */
    for (i = 0; i < SDRAM_SIZE; i += 4)
    {
        if (*(__IO uint32_t *)(SDRAM_BASE + i) != i)
        {
#if 1
            while (1);
#else
            os_kprintf("SDRAM test failed!\r\n");
            return OS_ERROR;
#endif
        }
    }

    os_kprintf("SDRAM test success!\r\n");
    return OS_EOK;
}

int SDRAM_Init(void)
{
    os_kprintf("sdram init success, mapped at 0x%X, size is %d bytes, data width is %d\r\n",
              SDRAM_BASE,
              SDRAM_SIZE,
              SDRAM_DATA_WIDTH);

    MPU_Config();

    sdram_test();
    
    return 0;
}

