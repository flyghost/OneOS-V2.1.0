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
 * @file        drv_qspi.h
 *
 * @brief       This file implements QSPI driver for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_QSPI_H_
#define __DRV_QSPI_H_
#include <os_task.h>

os_err_t stm32_qspi_bus_attach_device(const char *bus_name,
                                      const char *device_name,
                                      os_uint32_t pin,
                                      os_uint8_t  data_line_width,
                                      void (*enter_qspi_mode)(),
                                      void (*exit_qspi_mode)());

#endif /* __DRV_QSPI_H_ */
