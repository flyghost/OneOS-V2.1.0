/**
 ***********************************************************************************************************************
 * Copyright (c) 2021, China Mobile Communications Group Co.,Ltd.
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
 * @file        ope_error.h
 * 
 * @brief       Error status definition 
 * 
 * @details     Define the error status supported by the system
 * 
 * @revision
 * Date         Author          Notes
 * 2021-04-29   HuSong          First Version
 ***********************************************************************************************************************
 */

#ifndef __OPE_ERROR_H__
#define __OPE_ERROR_H__
#include "ope_log.h"

#define OPE_EOK      0x10   /* The operation was successful with no errors */
#define OPE_ENULL    0x11   /* A pointer is empty */
#define OPE_ELOWER   0x12   /* Less than the lower limit */
#define OPE_EUPPER   0x13   /* Above the ceiling */
#define OPE_ESIZE    0x14   /* The size of the error */
#define OPE_EDIV     0x15   /* Division error */
#define OPE_EFUNC    0x16   /* Function running error */
#define OPE_EMATCH   0x17   /* The matching error */
#define OPE_EMODE    0x18   /* Modo error */
#define OPE_EERROR   0x19   /* Other errors */

/* Fault-tolerant macro function definition */
/* When the assert_content result is false, the function in which this macro resides returns */
#define OPE_ASSERT(assert_content, ret) \
    if (!(assert_content)) \
    { \
        OPE_LOG_E("Condition("#assert_content") is not met"); \
        return (ret); \
    }

#endif /* __OPE_ERROR_H__ */