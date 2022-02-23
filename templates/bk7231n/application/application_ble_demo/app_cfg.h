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
 * @file        app_cfg.h
 *
 * @brief       Define the micro used to configure stack.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#ifndef _APP_CFG_H_
#define _APP_CFG_H_

#include <stdarg.h>

#ifndef MYNEWT_VAL_BLE_PUBLIC_DEV_ADDR
#define MYNEWT_VAL_BLE_PUBLIC_DEV_ADDR (((uint8_t[6]){0xcc, 0xbb, 0xaa, 0x33, 0x22, 0x11}))
#endif

#ifndef MYNEWT_VAL_TIMER_5
#define MYNEWT_VAL_TIMER_5 (1)
#endif

#define MYNEWT_VAL_OS_CPUTIME_TIMER_NUM (5)


#define STR_NUM(s) #s
#define XSTR_NUM(s) STR_NUM(s)                   // 将宏s展开，然后字符串化
#define STR_LINE_NUM    XSTR_NUM(__LINE__)

#define PARA_1(X) (1)
#define PARA_2(X, ...) (2)
#define PARA_3(X, ...) (3)
#define PARA_4(X, ...) (4)
#define PARA_5(X, ...) (5)
#define PARA_6(X, ...) (6)
#define PARA_7(X, ...) (7)
#define PARA_8(X, ...) (8)
#define PARA_9(X, ...) (9)
#define PARA_10(X, ...) (10)

#define _GET_MACRO_PARAS_NUM(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, NAME, ...) NAME
#define GET_MACRO_PARAS_NUM(...) \
    _GET_MACRO_PARAS_NUM(__VA_ARGS__, PARA_10, PARA_9, PARA_8, PARA_7, \
              PARA_6, PARA_5, PARA_4, PARA_3, PARA_2, PARA_1)(__VA_ARGS__)

#if 0
#define STR_BUF_WRITE(str, ...)   do {  \
    int macro_paras_num = GET_MACRO_PARAS_NUM(__VA_ARGS__); \
    const char *strbuf = str " L" STR_LINE_NUM  "\n";             \
    str_buf_write(strbuf, macro_paras_num, ##__VA_ARGS__);    \
} while (0)
#else
#define STR_BUF_WRITE(str, ...)
#endif

void str_buf_write(char *str, unsigned char para_c, ...);

#endif
