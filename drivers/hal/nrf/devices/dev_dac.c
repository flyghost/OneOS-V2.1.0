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
 * @file        drv_dac.c
 *
 * @brief       This file implements dac driver for nrf
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifdef BSP_USING_DAC0
struct nrf5_dac_info dac0_info = {.dac_periph = DAC0, .pin = GPIO_PIN_4};
OS_HAL_DEVICE_DEFINE("DAC_Type", "dac0", dac0_info);
#endif

#ifdef BSP_USING_DAC1
struct nrf5_dac_info dac1_info = {.dac_periph = DAC1, .pin = GPIO_PIN_5};
OS_HAL_DEVICE_DEFINE("DAC_Type", "dac1", dac1_info);
#endif

