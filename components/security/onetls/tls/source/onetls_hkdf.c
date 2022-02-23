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
 * @file        onetls_hkdf.c
 *
 * @brief       onetls_hkdf functions.
 *
 * @revision
 * Date         Author          Notes
 * 2021-03-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "onetls_hkdf.h"
#include "onetls_crypto.h"
#include "onetls_util.h"

uint32_t onetls_hkdf_extract(uint32_t hash_id, 
                             const uint8_t *salt, 
                             uint32_t salt_len, 
                             const uint8_t *key, 
                             uint32_t key_len, 
                             uint8_t *out, 
                             uint32_t out_len)
{
    uint8_t hkdf_zero[ONETLS_MAX_MD_LEN] = { 0 };    
    if (salt == NULL) {
        salt = hkdf_zero;
        salt_len = onetls_hash_size(hash_id);
    }
    if (key == NULL) {
        key = hkdf_zero;
        key_len = onetls_hash_size(hash_id);
    }
    return onetls_hmac(hash_id, salt, salt_len, key, key_len, out, &out_len);
}

uint32_t onetls_hkdf_expand(uint32_t hash_id,
                            const uint8_t *info, 
                            uint32_t info_len,
                            const uint8_t *key, 
                            uint32_t key_len,
                            uint8_t *out, 
                            uint32_t out_len)
{
    uint32_t md_size = onetls_hash_size(hash_id);
    uint32_t done_len = 0;
    uint8_t pre_out[ONETLS_MAX_MD_LEN * 2] = { 0 };
    uint8_t pre_out_2[ONETLS_MAX_MD_LEN] = { 0 };
    uint8_t rtt = out_len / md_size;
    
    // 对齐
    if ((out_len % md_size) != 0) {
        rtt ++;    
    }

    for (uint8_t offset = 1; offset <= rtt; offset++) {
        if (done_len > 0) {
             memcpy(pre_out, out, done_len);
        }
        memcpy(pre_out + done_len, info, info_len);
        pre_out[info_len + done_len] = offset;

        onetls_hmac(hash_id, key, key_len, pre_out, info_len + done_len + 1, pre_out_2, &md_size);
        
        memcpy(out + done_len, pre_out_2, onetls_min(out_len, md_size));
        done_len += onetls_min(out_len, md_size);
    }
    return ONETLS_SUCCESS;
}

uint32_t onetls_hkdf_expand_label(uint32_t hash_id,
                                  const uint8_t *key, 
                                  uint32_t key_len, 
                                  const char *label, 
                                  uint32_t label_len, 
                                  const uint8_t *info, 
                                  uint32_t info_len, 
                                  uint8_t *out, 
                                  uint32_t out_len)
{
    const char *protocol_label = ONETLS_LABEL_TLS13;
    uint8_t hkdf_label[ONETLS_HKDF_MAX_INFO_LEN + ONETLS_MAX_MD_LEN + 4] = { 0 };
    uint8_t *p = hkdf_label;

    if (info_len > ONETLS_HKDF_MAX_INFO_LEN) {
        onetls_check_errlog(ONETLS_INVALID_PARA, "onetls_hkdf_expand_label %s-%d", label, label_len);
        return ONETLS_INVALID_PARA;
    }

    *p++ = (out_len & 0x0000ff00) >> 8;
    *p++ = (out_len & 0x000000ff);
    *p++ = 6 + label_len;
    memcpy(p, protocol_label, 6);
    p += 6;
    memcpy(p, label, label_len);
    p += label_len;
    *p++ = info_len;
    memcpy(p, info, info_len);
    p += info_len;
    return onetls_hkdf_expand(hash_id, hkdf_label, p - hkdf_label, key, key_len, out, out_len);
}

uint32_t onetls_hkdf_derive_secret(uint32_t hash_id,
                                   const char *label, 
                                   uint32_t label_len,
                                   const uint8_t *secret, 
                                   uint32_t secret_len,
                                   const uint8_t *in, 
                                   uint32_t in_len,
                                   uint8_t *out, 
                                   uint32_t out_len)
{
    uint32_t pre_out_len = ONETLS_MAX_MD_LEN;
    uint8_t pre_out[ONETLS_MAX_MD_LEN] = { 0 };
    
    onetls_hash(hash_id, in, in_len, pre_out, &pre_out_len);

    uint32_t ret = onetls_hkdf_expand_label(hash_id, secret, secret_len, label, label_len, pre_out, pre_out_len, out, out_len);
    onetls_check_errlog(ret, "onetls_hkdf_expand_label");
    return ret;
}

uint32_t onetls_derive_secret_key_iv_finish_sn(uint32_t hash_id,
                                               const char *label, 
                                               uint8_t *salt, uint32_t salt_len, 
                                               uint8_t *in_secret, uint32_t in_secret_len,
                                               uint8_t *out_secret, uint32_t out_secret_len,
                                               uint8_t *key, uint32_t key_len,
                                               uint8_t *iv, uint32_t iv_len,
                                               uint8_t *finish, uint32_t finish_len,
                                               uint8_t *sn_key, uint32_t sn_key_len)
{
    memset(out_secret, 0, out_secret_len);
    uint32_t ret = onetls_hkdf_expand_label(hash_id,
                                            in_secret, in_secret_len, 
                                            label, strlen(label), 
                                            salt, salt_len, 
                                            out_secret, out_secret_len);
    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "onetls_hkdf_expand_label");
        return ret;
    }

    if (key) {
        memset(key, 0, key_len);
        ret = onetls_hkdf_expand_label(hash_id,
                                       out_secret, out_secret_len, 
                                       ONETLS_LABEL_KEY, strlen(ONETLS_LABEL_KEY), NULL, 0, 
                                       key, key_len);
        onetls_check_errlog(ret, "key");
    }

    if (iv) {
        memset(iv, 0, iv_len);
        ret = onetls_hkdf_expand_label(hash_id, 
                                       out_secret, out_secret_len, 
                                       ONETLS_LABEL_IV, strlen(ONETLS_LABEL_IV), NULL, 0, 
                                       iv, iv_len);
        onetls_check_errlog(ret, "iv");
    }

    if (finish) {
        memset(finish, 0, finish_len);
        ret = onetls_hkdf_expand_label(hash_id, 
                                       out_secret, out_secret_len, 
                                       ONETLS_LABEL_FINISHED, strlen(ONETLS_LABEL_FINISHED), NULL, 0, 
                                       finish, finish_len);
        onetls_check_errlog(ret, "finish");
    }

    if (sn_key) {
        memset(sn_key, 0, sn_key_len);
        ret = onetls_hkdf_expand_label(hash_id,
                                       out_secret, out_secret_len, 
                                       ONETLS_LABEL_SN, strlen(ONETLS_LABEL_SN), NULL, 0, 
                                       sn_key, sn_key_len);
        onetls_check_errlog(ret, "sn");
    }
    return ONETLS_SUCCESS;
}
