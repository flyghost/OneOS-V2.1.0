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
 * @file        sdio_func_ids.h
 *
 * @brief       This file provides macro definition for sdio.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __SDIO_FUNC_IDS_H__
#define __SDIO_FUNC_IDS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Standard SDIO Function Interfaces */

#define SDIO_FUNC_CODE_NONE        0x00 /* Not a SDIO standard interface */
#define SDIO_FUNC_CODE_UART        0x01 /* SDIO Standard UART */
#define SDIO_FUNC_CODE_BT_A        0x02 /* SDIO Type-A for Bluetooth standard interface */
#define SDIO_FUNC_CODE_BT_B        0x03 /* SDIO Type-B for Bluetooth standard interface */
#define SDIO_FUNC_CODE_GPS         0x04 /* SDIO GPS standard interface */
#define SDIO_FUNC_CODE_CAMERA      0x05 /* SDIO Camera standard interface */
#define SDIO_FUNC_CODE_PHS         0x06 /* SDIO PHS standard interface */
#define SDIO_FUNC_CODE_WLAN        0x07 /* SDIO WLAN interface */
#define SDIO_FUNC_CODE_ATA         0x08 /* Embedded SDIO-ATA standard interface */

/* manufacturer id, product io */

#define SDIO_MANUFACTURER_ID_MARVELL    0x02df
#define SDIO_PRODUCT_ID_MARVELL_88W8686 0x9103

#ifdef __cplusplus
}
#endif

#endif
