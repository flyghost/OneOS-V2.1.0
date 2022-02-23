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
 * @file        onetls_util.c
 *
 * @brief       onetls_util functions.
 *
 * @revision
 * Date         Author          Notes
 * 2021-03-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#include <sys/time.h>

#include "onetls_util.h"
#include "onetls_lib.h"

#ifndef OS_USING_RTC
#include <os_clock.h>
#include <oneos_config.h>
#endif

uint32_t onetls_time()
{   
    return (uint32_t)time(0);
}

uint32_t onetls_ms_time()
{
#ifdef OS_USING_RTC
    struct timeval tv = { 0 };
    gettimeofday(&tv, NULL);

    return (uint32_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
#else
    return os_tick_get() * 1000 / OS_TICK_PER_SECOND;
#endif
}

// 获取errno
int onetls_sys_errno()
{    
    return errno;
}

#if defined ONETLS_DEBUG
void onetls_log_info(const char *info)
{
    printf("%s", info);
}

void onetls_log_errcode(const char *file, const int line, const uint32_t err_code, const char *format, ...)
{   
    if (err_code == ONETLS_SUCCESS) {
        return;
    }

    // 有兴趣的同学可以把时间戳加上
    char log_string[ONETLS_LOG_STRING_MAX_LEN + 1] = { 0 };
    int len = snprintf(log_string, ONETLS_LOG_STRING_MAX_LEN, "%s_%d ret:0x%x tick_ms:0x%x errno:%d info:", 
                       file, line, err_code, 
                       onetls_ms_time(), onetls_sys_errno());
    if (len <= 0) {
        return;
    }

    va_list vArgList;
    va_start(vArgList, format);
    (void)vsnprintf(log_string + len, ONETLS_LOG_STRING_MAX_LEN - len - 1, format, vArgList);
    va_end(vArgList);

    onetls_log_info(log_string);    
}

void onetls_log_dump(const char *info, uint8_t *data, uint32_t data_len)
{
    char log_string[ONETLS_LOG_STRING_MAX_LEN + 1] = { 0 };
    snprintf(log_string, ONETLS_LOG_STRING_MAX_LEN, "%-32s  data_len:%d\r\n", info, data_len);
    onetls_log_info(log_string);

    for (uint32_t offset = 1; offset <= data_len; offset ++) {
        memset(log_string, 0, sizeof(log_string));
        snprintf(log_string, ONETLS_LOG_STRING_MAX_LEN, "0x%02x   ", data[offset - 1]);
        onetls_log_info(log_string);       

        // 换行
        if (offset % 8 == 0) {
            onetls_log_info("\r\n");
        }
    }
    onetls_log_info("\r\n\r\n");
}
#endif    

static void *onetls_malloc_sys(uint32_t size)
{
    uint32_t new_size = sizeof(onetls_slice_head) + size;
    uint8_t *ptr = (uint8_t *)malloc(new_size);
    if (ptr == NULL) {
        return NULL;
    }
    memset(ptr, 0, new_size);

    onetls_slice_head *head = (onetls_slice_head *)ptr;
    head->magic = ONETLS_MEM_MAGIC_HEAD;
    head->data_len = size;
    return (void*)((uint8_t*)ptr + sizeof(onetls_slice_head));
}

static void onetls_free_sys(void *p)
{
    onetls_slice_head *head = (onetls_slice_head *)((uint8_t*)p - sizeof(onetls_slice_head));
    if (head->magic != ONETLS_MEM_MAGIC_HEAD) {
        onetls_check_errlog(ONETLS_INNER_SYSERR, "onetls_free_sys");
    }

    memset(head, 0, head->data_len + sizeof(onetls_slice_head));
    free(head);
}

void *onetls_malloc(uint32_t size)
{
    if (size == 0) {
        return NULL;
    }
    uint8_t pack_len = size % 4;
    return onetls_malloc_sys((pack_len > 0) ? (size + 4 - pack_len) : size);
}

void onetls_free(void *ptr)
{
    if (ptr == NULL) {
        return;
    }
    onetls_free_sys(ptr);
}

uint8_t onetls_check_data_empty(const uint8_t *data, uint32_t len)
{
    for (uint32_t offset = 0; offset < len; offset++) {
        if (data[offset] != 0) {
            return 0;
        }
    }
    return 1;
}

void onetls_seq_num_add_ex(uint8_t *seq, uint32_t seq_size)
{
    for (uint8_t loop = seq_size; loop > 0; loop--) {
        seq[loop - 1] ++;
        if (seq[loop - 1] != 0) {
            break;
        }
    }
}

void onetls_seq_num_add(onetls_ctx *ctx, uint8_t sending)
{
    uint32_t seq_size = 8;
    uint8_t *seq = sending ? 
                    ctx->security.wr_seq.seqence_number :
                    ctx->security.rd_seq.seqence_number;
    onetls_seq_num_add_ex(seq, seq_size);
}

uint32_t onetls_get_u8(const uint8_t *p, uint8_t *b)
{
    *b = p[0];
    return 1;
}

uint32_t onetls_get_u16(const uint8_t *p, uint16_t *s)
{
    *s = p[0];
    *s <<= 8;

    *s += p[1];
    return 2;
}

uint32_t onetls_get_u24(const uint8_t *p, uint32_t *d)
{
    *d = p[0];
    *d <<= 8;

    *d += p[1];
    *d <<= 8;

    *d += p[2];
    return 3;
}

uint32_t onetls_get_u32(const uint8_t *p, uint32_t *d)
{
    *d = p[0];
    *d <<= 8;

    *d += p[1];
    *d <<= 8;

    *d += p[2];
    *d <<= 8;

    *d += p[3];
    return 4;
}

uint32_t onetls_get_u48(const uint8_t *p, uint64_t *ll)
{
    *ll = p[0];
    *ll <<= 8;

    *ll += p[1];
    *ll <<= 8;

    *ll += p[2];
    *ll <<= 8;
    
    *ll += p[3];
    *ll <<= 8;

    *ll += p[4];
    *ll <<= 8;

    *ll += p[5];
    return 6;
}

uint32_t onetls_get_u64(const uint8_t *p, uint64_t *ll)
{
    *ll = p[0];
    *ll <<= 8;

    *ll += p[1];
    *ll <<= 8;

    *ll += p[2];
    *ll <<= 8;
    
    *ll += p[3];
    *ll <<= 8;

    *ll += p[4];
    *ll <<= 8;

    *ll += p[5];
    *ll <<= 8;

    *ll += p[6];
    *ll <<= 8;    

    *ll += p[7];
    return 8;
}

uint32_t onetls_get_data(const uint8_t *p, uint8_t *data, uint32_t data_len)
{
    memcpy(data, p, data_len);
    return data_len;
}

uint32_t onetls_put_u8(uint8_t *p, uint8_t byte)
{
    p[0] = byte;
    return 1;
}

uint32_t onetls_put_u16(uint8_t *p, uint16_t word)
{
    p[0] = (word >> 8) & 0xff;
    p[1] = (word)      & 0xff;
    return 2;
}

uint32_t onetls_put_u24(uint8_t *p, uint32_t dword)
{
    p[0] = (dword >> 16) & 0xff;
    p[1] = (dword >> 8)  & 0xff;
    p[2] = (dword)       & 0xff;
    return 3;
}

uint32_t onetls_put_u32(uint8_t *p, uint32_t dword)
{
    p[0] = (dword >> 24) & 0xff;
    p[1] = (dword >> 16) & 0xff;
    p[2] = (dword >> 8)  & 0xff;
    p[3] = (dword)       & 0xff;
    return 4;
}

uint32_t onetls_put_u48(uint8_t *p, uint64_t ddword)
{
    p[0] = (ddword >> 40) & 0xff;
    p[1] = (ddword >> 32) & 0xff;
    p[2] = (ddword >> 24) & 0xff;
    p[3] = (ddword >> 16) & 0xff;
    p[4] = (ddword >> 8)  & 0xff;
    p[5] = (ddword)       & 0xff;
    return 6;
}

uint32_t onetls_put_u64(uint8_t *p, uint64_t ddword)
{
    p[0] = (ddword >> 56) & 0xff;
    p[1] = (ddword >> 48) & 0xff;
    p[2] = (ddword >> 40) & 0xff;
    p[3] = (ddword >> 32) & 0xff;
    p[4] = (ddword >> 24) & 0xff;
    p[5] = (ddword >> 16) & 0xff;
    p[6] = (ddword >> 8)  & 0xff;
    p[7] = (ddword)       & 0xff;    
    return 8;
}

uint32_t onetls_put_data(uint8_t *p, const uint8_t *data, uint32_t data_len)
{
    memcpy(p, data, data_len);
    return data_len;
}
