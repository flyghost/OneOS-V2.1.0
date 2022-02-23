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
 * @file        onetls_util.h
 *
 * @brief       onetls_util header file.
 *
 * @revision
 * Date         Author          Notes
 * 2021-03-16   OneOS Team      First Version.
 ***********************************************************************************************************************
 */

#ifndef __ONETLS_UTIL_H__
#define __ONETLS_UTIL_H__
#include "onetls.h"

#define ONETLS_MEM_PADDING_LEN (4)

#define ONETLS_MEM_MAGIC_HEAD 0xabababab

typedef struct {
    uint32_t magic;     // 魔法数的意义在于校验操作的合法性，原则上可以加上内存保护，做内存监控
    uint32_t data_len;  // 数据块长度
//  uint8_t  padding[ONETLS_MEM_PADDING_LEN];   // debug版本可以用来调测内存保护
} onetls_slice_head;

// 配置单条日志最大长度
#define ONETLS_LOG_STRING_MAX_LEN 255

// 大小比较
#define onetls_min(a, b) ((a) > (b)) ? (b) : (a)
#define onetls_max(a, b) ((a) > (b)) ? (a) : (b)
#define onetls_abs(a, b) ((a) > (b)) ? ((a) - (b)) : ((b) - (a))

#if defined ONETLS_DEBUG

    // 输出二进制信息
    void onetls_log_dump(const char *info, uint8_t *data, uint32_t data_len);

    // 输出带错误码信息
    void onetls_log_errcode(const char *file, const int line, const uint32_t err_code, const char *format, ...);

    // check日志
    #define onetls_check_errlog(err_code, format, ...) \
        onetls_log_errcode(__FILE__, __LINE__, (uint32_t)(err_code), format"\r\n", ##__VA_ARGS__)

#else 
    #define onetls_log_dump(a, b, c)
    #define onetls_check_errlog(err_code, format, ...) ((void)err_code)
#endif

// 获取系统的errno
int onetls_sys_errno(void);

// 获取当前时间
uint32_t onetls_time(void);
uint32_t onetls_ms_time(void);

void *onetls_malloc(uint32_t size);
void  onetls_free(void *ptr);

uint8_t onetls_check_data_empty(const uint8_t *data, uint32_t len);

void onetls_seq_num_add(onetls_ctx *ctx, uint8_t sending);

uint32_t onetls_get_u8(const uint8_t *p, uint8_t *b);
uint32_t onetls_get_u16(const uint8_t *p, uint16_t *s);
uint32_t onetls_get_u24(const uint8_t *p, uint32_t *d);
uint32_t onetls_get_u32(const uint8_t *p, uint32_t *d);
uint32_t onetls_get_u48(const uint8_t *p, uint64_t *d);
uint32_t onetls_get_u64(const uint8_t *p, uint64_t *d);
uint32_t onetls_get_data(const uint8_t *p, uint8_t *data, uint32_t data_len);

uint32_t onetls_put_u8(uint8_t *p, uint8_t byte);
uint32_t onetls_put_u16(uint8_t *p, uint16_t word);
uint32_t onetls_put_u24(uint8_t *p, uint32_t dword);
uint32_t onetls_put_u32(uint8_t *p, uint32_t dword);
uint32_t onetls_put_u48(uint8_t *p, uint64_t ddword);
uint32_t onetls_put_u64(uint8_t *p, uint64_t ddword);
uint32_t onetls_put_data(uint8_t *p, const uint8_t *data, uint32_t data_len);
#endif
