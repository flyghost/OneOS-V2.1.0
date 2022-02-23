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
* @file        mq_tls_onetls.h
*
* @brief       mq_tls_onetls header file.
*
* @revision
* Date         Author          Notes
* 2021-04-27   XieLi           First Version
***********************************************************************************************************************
*/

#ifndef __MQ_TLS_ONETLS_H_
#define __MQ_TLS_ONETLS_H_

#ifdef __cplusplus
extern "C"{
#endif

#ifndef MQTT_USING_ONETLS
#include <onetls.h>


/**
***********************************************************************************************************************
* @struct      mq_tls_session
*
* @brief       mqtt tls session.
*
***********************************************************************************************************************
*/
struct mq_tls_session
{
    int sock;           /* sock fd */
    onetls_ctx	*ctx;	/* onetls context */
};
typedef struct mq_tls_session mq_tls_session_t;

mq_tls_session_t *mq_tls_network_onetls_establish(const char *addr, uint16_t port, const uint8_t *psk_identity, 
                                                uint16_t psk_identity_len, onetls_psk_callback psk_cb);

int mq_tls_network_onetls_close(mq_tls_session_t *session);                                                
int mq_tls_network_onetls_read(mq_tls_session_t *tls_session, unsigned char *buf, size_t len);
int mq_tls_network_onetls_write(mq_tls_session_t *tls_session, const unsigned char *buf , size_t len);

#endif

#ifdef __cplusplus
}
#endif

#endif  /* __MQ_TLS_ONETLS_H_ */
