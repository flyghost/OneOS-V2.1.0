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
 * @file        onetls_crypto.h
 *
 * @brief       onetls_crypto header file.
 *
 * @revision
 * Date         Author          Notes
 * 2021-03-16   OneOS Team      First Version.
 ***********************************************************************************************************************
 */

#ifndef __ONETLS_CRYPTO_H__
#define __ONETLS_CRYPTO_H__
#include "onetls.h"

// 目前一整套都是基于PSA/mbedtls来实现的
#include "psa/crypto.h"
#include "mbedtls/entropy.h"
#include "mbedtls/aes.h"

#define ONETLS_DH_PB_KEY_LEN (133)

#define ONETLS_MAX_MD_LEN 64
#define ONETLS_MAX_IV_LEN 16

#define ONETLS_ATTR_CIPHER 1
#define ONETLS_ATTR_GROUPS 2
#define ONETLS_ATTR_SIGN   3

typedef struct {
    uint16_t    iana_id;    
} onetls_sign_alg;

typedef struct {
    uint16_t    iana_id;
    uint16_t    key_type;       // 套件类型    
    uint32_t    hash_alg;       // hkdf的算法    
    uint32_t    key_alg;        // 算法ID    
    uint8_t     key_len;        // 秘钥的长度
    uint8_t     iv_len;         // iv的长度
    uint8_t     tag_len;        // tag长度
} onetls_cipher;

typedef struct {
    uint16_t    iana_id;
    uint16_t    key_bits;
    uint32_t    kex;    
    uint8_t     curve;
    uint8_t     public_key_len;
} onetls_dh_groups;

typedef struct {
    const onetls_dh_groups *group;
    psa_key_handle_t private_key_index; // dh 私钥索引(openssl evp的话，这里需要改造成void *)
    uint8_t          public_key[ONETLS_DH_PB_KEY_LEN];        // dh 公钥
} onetls_key_share;

const onetls_cipher *onetls_get_cipher_byid(uint16_t id);

void onetls_crypto_init(void);

void onetls_get_rnd(uint8_t *data, uint32_t len);

uint32_t onetls_hash_size(uint32_t hash_id);

uint32_t onetls_hash(uint32_t hash_id, 
                     const uint8_t *in, 
                     uint32_t in_len, 
                     uint8_t *out, 
                     uint32_t *out_len);

uint32_t onetls_hmac(uint32_t hash_id,
                     const uint8_t *key, 
                     uint32_t key_len,
                     const uint8_t *in, 
                     uint32_t in_len,
                     uint8_t *out, 
                     uint32_t *out_len);

uint32_t onetls_tls13_default_hkdf_hash(void);

uint32_t onetls_gen_kex_with_pub_key(onetls_ctx *ctx, onetls_key_share *key_share, const uint8_t* pub_key, uint32_t pub_len);
void onetls_del_key_share(onetls_key_share *key_share);
uint32_t onetls_new_keyshare(onetls_ctx *ctx);

uint32_t onetls_crypto_create_ctx(psa_key_handle_t *ctx, const onetls_cipher *cipher, uint8_t *key, uint8_t usage);
void onetls_crypto_delete_ctx(psa_key_handle_t *ctx);

uint32_t onetls_aead_decrypt(onetls_ctx *ctx, 
                             uint8_t *nonce, 
                             uint8_t *add, uint32_t add_len, 
                             uint8_t *in, uint32_t in_len, 
                             uint8_t *mt, 
                             uint8_t *out, uint32_t *out_len);
uint32_t onetls_aead_encrypt(onetls_ctx *ctx, 
                             uint8_t *nonce, 
                             uint8_t *add, uint32_t add_len, 
                             uint8_t *in, uint32_t in_len, 
                             uint8_t *mt, 
                             uint8_t *out, uint32_t *out_len);

uint32_t onetls_aes_ecb(uint8_t en, uint8_t *key, uint32_t key_len, uint8_t *input, uint8_t *output);

void onetls_construct_client_hello_attr(uint8_t type, uint8_t *packet, uint32_t *length);
#endif
