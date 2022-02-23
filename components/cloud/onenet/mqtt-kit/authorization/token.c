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
 * @file        token.c
 * 
 * @brief       Supply functions to calculate OneNET-MQTTS token, which use in device register and conncet.  
 * 
 * @details     
 * 
 * @revision
 * Date         Author          Notes
 * 2020-06-08   OneOs Team      First Version
 ***********************************************************************************************************************
 */

#include <stdio.h>
#include <string.h>
#include "onenet_base64.h"
#include "hmac_sha1.h"

#define METHOD "sha1"

static unsigned char ota_url_encode(char *sign)
{
    char          sign_t[40];
    unsigned char i = 0;
    unsigned char j = 0;
    unsigned char sign_len = strlen(sign);

    if (sign == (void *)0 || sign_len < 28)
    {
        return 1;
    }

    for (; i < sign_len; i++)
    {
        sign_t[i] = sign[i];
        sign[i]   = 0;
    }
    sign_t[i] = 0;

    for (i = 0, j = 0; i < sign_len; i++)
    {
        switch (sign_t[i])
        {
        case '+':
            strcat(sign + j, "%2B");
            j += 3;
            break;
        case ' ':
            strcat(sign + j, "%20");
            j += 3;
            break;
        case '/':
            strcat(sign + j, "%2F");
            j += 3;
            break;
        case '?':
            strcat(sign + j, "%3F");
            j += 3;
            break;
        case '%':
            strcat(sign + j, "%25");
            j += 3;
            break;
        case '#':
            strcat(sign + j, "%23");
            j += 3;
            break;
        case '&':
            strcat(sign + j, "%26");
            j += 3;
            break;
        case '=':
            strcat(sign + j, "%3D");
            j += 3;
            break;
        default:
            sign[j] = sign_t[i];
            j++;
            break;
        }
    }

    sign[j] = 0;

    return 0;
}

/**
***********************************************************************************************************************
* @brief           This function will calculate OneNET-MQTTS authorization.
*
* @details
*
* @attention       When "et" set less than present time, OneNET may decide authorization expire.
*
* @param[in]       ver                     Must be "2018-10-31".
* @param[in]       res                     Product id, get in OneNET web, when user has registered with MQTT.
* @param[in]       et                      Utc expiration time.
* @param[in]       access_key              Product access_key-API, device key-device connect.
* @param[in]       dev_name                The device name, user defined.
* @param[in,out]   authorization_buf       The pointer of token buffer.
* @param[out]      authorization_buf_len   The token buffer length.
* @param[in]       flag                    1-API, 0-device connect.
*
* @return          Return calculate status.
* @retval          0                       authorization calculate success.
* @retval          1                       authorization calculate failure.
***********************************************************************************************************************
*/
int onenet_authorization(char          *ver,
                         char          *res,
                         unsigned int   et,
                         char          *access_key,
                         char          *dev_name,
                         char          *authorization_buf,
                         unsigned short authorization_buf_len,
                         _Bool          flag)
{
    char sign_buf[64];
    char hmac_sha1_buf[64];
    char access_key_base64[64];
    char string_for_signature[72];

    size_t olen = 0;
    /* check parameter */
    if (ver == (void *)0 || res == (void *)0 || et < 1582539158 || access_key == (void *)0 ||
        authorization_buf == (void *)0 || authorization_buf_len < 120)
    {
        return 1;
    }

    /* access_key base64 decode */
    memset(access_key_base64, 0, sizeof(access_key_base64));
    BASE64_Decode((unsigned char *)access_key_base64,
                  sizeof(access_key_base64),
                  &olen,
                  (unsigned char *)access_key,
                  strlen(access_key));
    /* LOG_EXT_D("access_key_base64: %s", access_key_base64); */

    /* calculate string_for_signature */
    memset(string_for_signature, 0, sizeof(string_for_signature));
    if (flag)
    {
        snprintf(string_for_signature, sizeof(string_for_signature), "%d\n%s\nproducts/%s\n%s", et, METHOD, res, ver);
    }
    else
    {
        snprintf(string_for_signature,
                 sizeof(string_for_signature),
                 "%d\n%s\nproducts/%s/devices/%s\n%s",
                 et,
                 METHOD,
                 res,
                 dev_name,
                 ver);
    }
    /*LOG_EXT_D("string_for_signature: %s", string_for_signature);*/

    /* encode */
    memset(hmac_sha1_buf, 0, sizeof(hmac_sha1_buf));
    hmac_sha1((unsigned char *)access_key_base64,
              strlen(access_key_base64),
              (unsigned char *)string_for_signature,
              strlen(string_for_signature),
              (unsigned char *)hmac_sha1_buf);
    /* LOG_EXT_D("hmac_sha1_buf: %s", hmac_sha1_buf); */

    /* encode result base64 encode */
    olen = 0;
    memset(sign_buf, 0, sizeof(sign_buf));
    BASE64_Encode((unsigned char *)sign_buf, sizeof(sign_buf), &olen, (unsigned char *)hmac_sha1_buf, 20);

    /* base64 result url encode */
    ota_url_encode(sign_buf);
    /* LOG_EXT_D("sign_buf: %s", sign_buf); */

    /* calculate token */
    if (flag)
    {
        snprintf(authorization_buf,
                 authorization_buf_len,
                 "version=%s&res=products%%2F%s&et=%d&method=%s&sign=%s",
                 ver,
                 res,
                 et,
                 METHOD,
                 sign_buf);
    }
    else
    {
        snprintf(authorization_buf,
                 authorization_buf_len,
                 "version=%s&res=products%%2F%s%%2Fdevices%%2F%s&et=%d&method=%s&sign=%s",
                 ver,
                 res,
                 dev_name,
                 et,
                 METHOD,
                 sign_buf);
    }
    /*LOG_EXT_D("Token: %s", authorization_buf);*/

    return 0;
}
