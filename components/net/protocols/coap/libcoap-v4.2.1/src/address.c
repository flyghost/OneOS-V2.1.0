/* address.c -- representation of network addresses
 *
 * Copyright (C) 2015-2016,2019 Olaf Bergmann <bergmann@tzi.org>
 *
 * This file is part of the CoAP library libcoap. Please see
 * README for terms of use.
 */

#include "coap_internal.h"

#if !defined(WITH_CONTIKI) && !defined(WITH_LWIP)
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_WS2TCPIP_H
#include <ws2tcpip.h>
#endif

#ifdef RIOT_VERSION
/* FIXME */
#define IN_MULTICAST(Address) (0)
#endif /* RIOT_VERSION */

#ifndef IN6_IS_ADDR_MULTICAST
#define IN6_IS_ADDR_MULTICAST(a) (((uint8_t *) (a))[0] == 0xff)
#endif

int
coap_address_equals(const coap_address_t *a, const coap_address_t *b) {
  assert(a); assert(b);

  if (a->size != b->size || a->addr.sa.sa_family != b->addr.sa.sa_family)
    return 0;

  /* need to compare only relevant parts of sockaddr_in6 */
 switch (a->addr.sa.sa_family) {
 case AF_INET:
   return
     a->addr.sin.sin_port == b->addr.sin.sin_port &&
     memcmp(&a->addr.sin.sin_addr, &b->addr.sin.sin_addr,
            sizeof(struct in_addr)) == 0;
#ifdef HAS_IPV6  
 case AF_INET6:
   return a->addr.sin6.sin6_port == b->addr.sin6.sin6_port &&
     memcmp(&a->addr.sin6.sin6_addr, &b->addr.sin6.sin6_addr,
            sizeof(struct in6_addr)) == 0;
#endif
 default: /* fall through and signal error */
   ;
 }
 return 0;
}

int coap_is_mcast(const coap_address_t *a) {
  if (!a)
    return 0;

 switch (a->addr.sa.sa_family) {
 case AF_INET:
   return IN_MULTICAST(ntohl(a->addr.sin.sin_addr.s_addr));
#ifdef HAS_IPV6  
 case  AF_INET6:
   return IN6_IS_ADDR_MULTICAST(&a->addr.sin6.sin6_addr);
#endif
 default:  /* fall through and signal error */
   ;
  }
 return 0;
}

#endif /* !defined(WITH_CONTIKI) && !defined(WITH_LWIP) */

void coap_address_init(coap_address_t *addr) {
  assert(addr);
  memset(addr, 0, sizeof(coap_address_t));
#if !defined(WITH_LWIP) && !defined(WITH_CONTIKI)
  /* lwip and Contiki have constant address sizes and don't need the .size part */
  addr->size = sizeof(addr->addr);
#endif
}

