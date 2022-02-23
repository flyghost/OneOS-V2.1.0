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
 * \@file        coap_config.h
 *
 * \@brief       coap config file
 *
 * \@details     
 *
 * \@revision
 * Date         Author          Notes
 * 2020-06-08   OneOS Team      first version
 ***********************************************************************************************************************
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "oneos_config.h"
//#include "os_kernel.h"
#include "sys/errno.h"
#include "sys/ioctl.h"
//#include "libc_fdset.h"
#include "vfs_posix.h"
#include "vfs_select.h"

#define USE_ONEOS
#define HAVE_NETDB_H
//#define HAVE_NETINET_IN_H

#define PACKAGE_NAME    "libcoap-oneos"
#define PACKAGE_VERSION "?"
#define PACKAGE_STRING  PACKAGE_NAME PACKAGE_VERSION

#define assert(x) LWIP_ASSERT("CoAP assert failed", x)

/* it's just provided by libc. i hope we don't get too many of those, as
 * actually we'd need autotools again to find out what environment we're
 * building in */

#if defined(__GNUC__)
#define HAVE_STRNLEN
#endif

#define HAVE_LIMITS_H

#define HAVE_MALLOC
#define HAVE_SYS_SOCKET_H
#define HAVE_TIME_H
/* #define COAP_CLOCK*/

#define NO_DECLTYPE
/* #define USE_COAP_SERVER*/

/*#define HAS_IPV6 */

/*#define COAP_USE_DTLS 1*/

/*#define COAP_USE_TLS 1*/

#define COAP_RESOURCES_NOHASH

#define WITH_POSIX 
//#define WITH_LWIP 

/*#define COAP_FROM_HY*/
/*#define COAP_FOR_ANDLINK*/

#endif /* _CONFIG_H_ */
