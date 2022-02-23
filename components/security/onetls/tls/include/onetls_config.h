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
 * @file        onetls_config.h
 *
 * @brief       onetls_config header file.
 *
 * @revision
 * Date         Author          Notes
 * 2021-03-16   OneOS Team      First Version.
 ***********************************************************************************************************************
 */

#ifndef __ONETLS_CONFIG_H__
#define __ONETLS_CONFIG_H__
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* 集成进OneOS，OneTLS协议栈可以独立运行 */
#define ONETLS_EMBED_ONEOS

#ifdef ONETLS_EMBED_ONEOS
#include <oneos_config.h>
#else
/* Debug 标识，以debug模式运行，可以控制日志打印、断言、Dump等调测功能 */
#define ONETLS_DEBUG

/* 是否启动DTLS协议 */
#define ONETLS_RUN_DTLS

/* 最大记录层报文长度
    对于TLS协议：如果能同时控制通信两端的最大报文，建议配置为合理的业务报文大小，否则，建议配置为RFC规范的默认值16384；
    对于DTLS协议：配置为MTU的大小即可
    建议配置：enum(512 1024 2048 4096 8192 16384)
*/
#define ONETLS_MAX_RECORD_PACKET_LEN  1024

/* 配置支持会话恢复 */
#define ONETLS_SUPPORT_RESUMPTION

/*配置支持0RTT数据 */
#define ONETLS_SUPPORT_0RTT

#endif


/**********************************************************************************************************************/
/* 校验配置项 */
#if ((ONETLS_MAX_RECORD_PACKET_LEN < 512) || (ONETLS_MAX_RECORD_PACKET_LEN > 16384))
    #error("onetls invalid max record packet length!")
#endif

#if defined ONETLS_SUPPORT_0RTT // 定义了0RTT
    #if !defined ONETLS_SUPPORT_RESUMPTION  // 却没有定义会话恢复
        #error("0rtt mode depend on resumption!")
    #endif
#endif

//配置算法套件
#if ((!defined ONETLS_CIPHER_AES_128_GCM_SHA256) &&      \
    (!defined ONETLS_CIPHER_CHACHA20_POLY1305_SHA256) && \
    (!defined ONETLS_CIPHER_AES_128_CCM_SHA256) &&       \
    (!defined ONETLS_CIPHER_AES_128_CCM_8_SHA256)) ||    \
    ((!defined ONETLS_ECP_DP_SECP256R1_ENABLED) &&       \
    (!defined ONETLS_ECP_DP_SECP384R1_ENABLED) &&        \
    (!defined ONETLS_ECP_DP_SECP521R1_ENABLED) &&        \
    (!defined ONETLS_ECP_DP_CURVE25519_ENABLED) )
    #error("select at least one cipher and one curves")
#endif
//cipher
#if defined ONETLS_CIPHER_DEFAULT
#define ONETLS_CIPHER_AES_128_GCM_SHA256
//Mbedtls不支持sha384
//#define ONETLS_CIPHER_AES_256_GCM_SHA384
#define ONETLS_CIPHER_CHACHA20_POLY1305_SHA256
#define ONETLS_CIPHER_AES_128_CCM_SHA256
#define ONETLS_CIPHER_AES_128_CCM_8_SHA256
#endif

#if defined(ONETLS_CIPHER_AES_128_GCM_SHA256)
#define MBEDTLS_CIPHER_C
#define MBEDTLS_AES_C
#define MBEDTLS_GCM_C
#define MBEDTLS_SHA256_C
#endif

//TODO：Mbedtls不支持sha384
#if defined(ONETLS_CIPHER_AES_256_GCM_SHA384)
#define MBEDTLS_CIPHER_C
#define MBEDTLS_AES_C
#define MBEDTLS_GCM_C
#endif

#if defined(ONETLS_CIPHER_CHACHA20_POLY1305_SHA256)
#define MBEDTLS_CHACHA20_C
#define MBEDTLS_AES_C
#define MBEDTLS_POLY1305_C
#define MBEDTLS_CHACHAPOLY_C
#define MBEDTLS_SHA256_C
#define MBEDTLS_CIPHER_C
#endif

#if defined(ONETLS_CIPHER_AES_128_CCM_SHA256)
#define MBEDTLS_CIPHER_C
#define MBEDTLS_AES_C
#define MBEDTLS_CCM_C
#define MBEDTLS_SHA256_C
#endif

#if defined(ONETLS_CIPHER_AES_128_CCM_8_SHA256)
#define MBEDTLS_CIPHER_C
#define MBEDTLS_AES_C
#define MBEDTLS_CCM_C
#define MBEDTLS_SHA256_C
#endif

//椭圆曲线
#if defined(ONETLS_ECP_DP_SECP256R1_ENABLED)
    #define MBEDTLS_ECP_DP_SECP256R1_ENABLED
#endif
#if defined(ONETLS_ECP_DP_SECP384R1_ENABLED)
    #define MBEDTLS_ECP_DP_SECP384R1_ENABLED
#endif
#if defined(ONETLS_ECP_DP_SECP521R1_ENABLED)
    #define MBEDTLS_ECP_DP_SECP521R1_ENABLED
#endif
#if defined(ONETLS_ECP_DP_CURVE25519_ENABLED)
    #define MBEDTLS_ECP_DP_CURVE25519_ENABLED
#endif

/* mbed TLS modules */
/* System support */
#define MBEDTLS_HAVE_ASM
#define MBEDTLS_HAVE_TIME
/* 0. Entropy and Random */
// #define MBEDTLS_PSA_INJECT_ENTROPY
#define MBEDTLS_NO_PLATFORM_ENTROPY
#define MBEDTLS_NO_DEFAULT_ENTROPY_SOURCES
#define MBEDTLS_ENTROPY_C
#define MBEDTLS_CTR_DRBG_C
#define MBEDTLS_MD_C
#define MBEDTLS_HMAC_DRBG_C
/* 1. Key exchange */
#if ((defined ONETLS_ECP_DP_SECP256R1_ENABLED) ||       \
    (defined ONETLS_ECP_DP_SECP384R1_ENABLED) ||        \
    (defined ONETLS_ECP_DP_SECP521R1_ENABLED) ||        \
    (defined ONETLS_ECP_DP_CURVE25519_ENABLED) )
#define MBEDTLS_ECP_C
#define MBEDTLS_ECDH_C
#endif
#define MBEDTLS_DHM_C
#define MBEDTLS_BIGNUM_C
/* 2. PSA */
#define MBEDTLS_PSA_CRYPTO_C

#include "mbedtls/check_config.h"

#endif
