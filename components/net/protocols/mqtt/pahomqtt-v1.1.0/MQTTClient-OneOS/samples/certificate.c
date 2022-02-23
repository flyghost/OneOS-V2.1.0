/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the \"License\ you may not use this file except in compliance with 
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on 
 * an \"AS IS\" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the 
 * specific language governing permissions and limitations under the License.
 *
 * \@file        MQTTOneOS.c
 *
 * \@brief       socket port file for mqtt
 *
 * \@details     
 *
 * \@revision
 * Date         Author          Notes
 * 2020-06-08   OneOS Team      first version
 ***********************************************************************************************************************
 */

#include <stdlib.h>
#include <oneos_config.h>

#ifdef MQTT_USING_TLS
const char g_certificate[] =
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIB2zCCAUQCCQDXPSD/AG+GSjANBgkqhkiG9w0BAQUFADAyMTAwLgYDVQQKDCdU\r\n"
    "TFMgUHJvamVjdCBEb2RneSBDZXJ0aWZpY2F0ZSBBdXRob3JpdHkwHhcNMjAwNjE2\r\n"
    "MDIzNTU5WhcNMzQwMjIzMDIzNTU5WjAyMTAwLgYDVQQKDCdUTFMgUHJvamVjdCBE\r\n"
    "b2RneSBDZXJ0aWZpY2F0ZSBBdXRob3JpdHkwgZ8wDQYJKoZIhvcNAQEBBQADgY0A\r\n"
    "MIGJAoGBANCX40U632pEX6ejNCFlD+6+ck3QTRqwtmR/WsHIBVcZL26VJU6WWKYT\r\n"
    "9/OP9rMlqhIRpKBOPRuykhU6XASi5dJQZqc59aXLjj8LRLuzNoI+KsRRnSTOqlNL\r\n"
    "C30ccP2NEJCp7TKCOoQr7GRRBnTEgwgooVispIa9f5WdlVABFgQHAgMBAAEwDQYJ\r\n"
    "KoZIhvcNAQEFBQADgYEAhaLL6XH/egpIceAWlOLroZDokbdgFuzGMtBr/rXwdau6\r\n"
    "aWmNv5h5ej/x93do96s5kxi4VAhgegVVHyx8hIYpM/VbQIgcJIvDteRNlht1DNVa\r\n"
    "/HrIlJKmJSgOAtCgAsz8jeCUliWhSDW1uP5dQHJWQJLo00WAI19tW+mJnJzbnVE=\r\n"
    "-----END CERTIFICATE-----\r\n"

    ;
#else
const char *g_certificate = NULL;
#endif
