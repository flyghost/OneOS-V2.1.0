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
 * @file        onetls_hkdf.h
 *
 * @brief       onetls_hkdf header file.
 *
 * @revision
 * Date         Author          Notes
 * 2021-03-16   OneOS Team      First Version.
 ***********************************************************************************************************************
 */

#ifndef __ONETLS_HKDF_H__
#define __ONETLS_HKDF_H__
#include "onetls.h"

#define ONETLS_HKDF_MAX_INFO_LEN 255

#define ONETLS_LABEL_DERIVED "derived"
#define ONETLS_LABEL_TLS13 "tls13 "
#define ONETLS_LABEL_FINISHED "finished"
#define ONETLS_LABEL_KEY "key"
#define ONETLS_LABEL_IV "iv"
#define ONETLS_LABEL_SN "sn"
#define ONETLS_LABEL_EXT_BINDER "ext binder"
#define ONETLS_LABEL_RES_BINDER "res binder"
#define ONETLS_LABEL_EARLY_TRAFFIC "c e traffic"
#define ONETLS_LABEL_SERVER_HS_TRAFFIC "s hs traffic"
#define ONETLS_LABEL_CLIENT_HS_TRAFFIC "c hs traffic"
#define ONETLS_LABEL_SERVER_AP_TRAFFIC "s ap traffic"
#define ONETLS_LABEL_CLIENT_AP_TRAFFIC "c ap traffic"
#define ONETLS_LABEL_EXP_MASTER "exp master"
#define ONETLS_LABEL_EXP_MASTER_KEY "exporter"
#define ONETLS_LABEL_RES_MASTER "res master"
#define ONETLS_LABEL_RES_MASTER_KEY "resumption"
#define ONETLS_LABEL_TRAFFIC_UPDATE "traffic upd"


uint32_t onetls_hkdf_extract(uint32_t hash_id, 
                             const uint8_t *salt, 
                             uint32_t salt_len, 
                             const uint8_t *key, 
                             uint32_t key_len, 
                             uint8_t *out, 
                             uint32_t out_len);

uint32_t onetls_hkdf_expand(uint32_t hash_id,
                            const uint8_t *info, 
                            uint32_t info_len,
                            const uint8_t *key, 
                            uint32_t key_len,
                            uint8_t *out, 
                            uint32_t out_len);

uint32_t onetls_hkdf_expand_label(uint32_t hash_id,
                                  const uint8_t *key, 
                                  uint32_t key_len, 
                                  const char *label, 
                                  uint32_t label_len, 
                                  const uint8_t *info, 
                                  uint32_t info_len, 
                                  uint8_t *out, 
                                  uint32_t out_len);

uint32_t onetls_hkdf_derive_secret(uint32_t hash_id,
                                   const char *label, 
                                   uint32_t label_len,
                                   const uint8_t *secret, 
                                   uint32_t secret_len,
                                   const uint8_t *in, 
                                   uint32_t in_len,
                                   uint8_t *out, 
                                   uint32_t out_len);

uint32_t onetls_derive_secret_key_iv_finish_sn(uint32_t hash_id,
                                               const char *label, 
                                               uint8_t *salt, uint32_t salt_len, 
                                               uint8_t *in_secret, uint32_t in_secret_len,
                                               uint8_t *out_secret, uint32_t out_secret_len,
                                               uint8_t *key, uint32_t key_len,
                                               uint8_t *iv, uint32_t iv_len,
                                               uint8_t *finish, uint32_t finish_len,
                                               uint8_t *sn_key, uint32_t sn_key_len);
#endif
