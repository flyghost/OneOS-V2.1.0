/*
 * Copyright (c) 2013, Institute for Pervasive Computing, ETH Zurich
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *      An implementation of the Constrained Application Protocol (draft 12)
 * \author
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch>
 * \contributors
 *    David Navarro, Intel Corporation - Adapt to usage in liblwm2m
 */


#ifndef COAP_13_H_
#define COAP_13_H_

#include <stdint.h>
#include <stddef.h> /* for size_t */

typedef uint8_t coap_status_t;

//coap error code 
#define COAP_NO_ERROR                   (uint8_t)0x00
#define COAP_IGNORE                     (uint8_t)0x01

#define COAP_201_CREATED                (uint8_t)0x41
#define COAP_202_DELETED                (uint8_t)0x42
#define COAP_204_CHANGED                (uint8_t)0x44
#define COAP_205_CONTENT                (uint8_t)0x45
#define COAP_206_CONFORM                (uint8_t)0x46

#define COAP_231_CONTINUE               (uint8_t)0x5F
#define COAP_400_BAD_REQUEST            (uint8_t)0x80
#define COAP_401_UNAUTHORIZED           (uint8_t)0x81
#define COAP_402_BAD_OPTION             (uint8_t)0x82
#define COAP_403_FORBIDDEN              (uint8_t)0x83
#define COAP_404_NOT_FOUND              (uint8_t)0x84
#define COAP_405_METHOD_NOT_ALLOWED     (uint8_t)0x85
#define COAP_406_NOT_ACCEPTABLE         (uint8_t)0x86
#define COAP_408_REQ_ENTITY_INCOMPLETE  (uint8_t)0x88
#define COAP_412_PRECONDITION_FAILED    (uint8_t)0x8C
#ifndef COAP_413_ENTITY_TOO_LARGE
#define COAP_413_ENTITY_TOO_LARGE       (uint8_t)0x8F
#endif
#define COAP_500_INTERNAL_SERVER_ERROR  (uint8_t)0xA0
#define COAP_501_NOT_IMPLEMENTED        (uint8_t)0xA1
#define COAP_503_SERVICE_UNAVAILABLE    (uint8_t)0xA3
#define COAP_505_PROXYING_NOT_SUPPORTED (uint8_t)0xA5
#define COAP_UNKOWN_ERROR               (uint8_t)0xC0


#define STR_COAP_CODE(M)                                \
    ((M) == COAP_NO_ERROR ? "COAP_NO_ERROR" :      \
    ((M) == COAP_IGNORE ? "COAP_IGNORE" :      \
    ((M) == COAP_201_CREATED ? "COAP_201" :      \
    ((M) == COAP_202_DELETED ? "COAP_202" :      \
    ((M) == COAP_204_CHANGED ? "COAP_204" :  \
    ((M) == COAP_205_CONTENT ? "COAP_205" :        \
    ((M) == COAP_206_CONFORM ? "COAP_206" : \
    ((M) == COAP_231_CONTINUE ? "COAP_231" :      \
    ((M) == COAP_400_BAD_REQUEST ? "COAP_400" :      \
    ((M) == COAP_401_UNAUTHORIZED ? "COAP_401" :      \
    ((M) == COAP_402_BAD_OPTION ? "COAP_402" :      \
    ((M) == COAP_404_NOT_FOUND ? "COAP_404" :      \
    ((M) == COAP_405_METHOD_NOT_ALLOWED ? "COAP_405" :      \
    ((M) == COAP_406_NOT_ACCEPTABLE ? "COAP_406" :      \
    ((M) == COAP_408_REQ_ENTITY_INCOMPLETE ? "COAP_408" :      \
    ((M) == COAP_413_ENTITY_TOO_LARGE ? "COAP_413" :      \
    ((M) == COAP_500_INTERNAL_SERVER_ERROR ? "COAP_500" :      \
    ((M) == COAP_501_NOT_IMPLEMENTED ? "COAP_501" :      \
    ((M) == COAP_503_SERVICE_UNAVAILABLE ? "COAP_503" :      \
    ((M) == COAP_505_PROXYING_NOT_SUPPORTED ? "COAP_505" :      \
    "Unknown"))))))))))))))))))))



/*
 * The maximum buffer size that is provided for resource responses and must be respected due to the limited IP buffer.
 * Larger data must be handled by the resource and will be sent chunk-wise through a TCP stream or CoAP blocks.
 */
#ifndef REST_MAX_CHUNK_SIZE
#define REST_MAX_CHUNK_SIZE                  512
#endif

#define COAP_DEFAULT_MAX_AGE                 60
#define COAP_RESPONSE_TIMEOUT                4
//TODO add marco to modify
#define COAP_MAX_RETRANSMIT                  4
#define COAP_ACK_RANDOM_FACTOR               1.5

#define COAP_MAX_TRANSMIT_WAIT               ((COAP_RESPONSE_TIMEOUT * ( (1 << (COAP_MAX_RETRANSMIT + 1) ) - 1) * COAP_ACK_RANDOM_FACTOR))
#define COAP_MIN_TRANSMIT_WAIT               (COAP_RESPONSE_TIMEOUT * ( (1 << (COAP_MAX_RETRANSMIT + 1) ) - 1))

#define COAP_HEADER_LEN                      4 /* | version:0x03 type:0x0C tkl:0xF0 | code | mid:0x00FF | mid:0xFF00 | */
#define COAP_ETAG_LEN                        8 /* The maximum number of bytes for the ETag */
#define COAP_TOKEN_LEN                       8 /* The maximum number of bytes for the Token */
#define COAP_MAX_ACCEPT_NUM                  2 /* The maximum number of accept preferences to parse/store */

#define COAP_MAX_OPTION_HEADER_LEN           5

#define COAP_HEADER_VERSION_MASK             0xC0
#define COAP_HEADER_VERSION_POSITION         6
#define COAP_HEADER_TYPE_MASK                0x30
#define COAP_HEADER_TYPE_POSITION            4
#define COAP_HEADER_TOKEN_LEN_MASK           0x0F
#define COAP_HEADER_TOKEN_LEN_POSITION       0

#define COAP_HEADER_OPTION_DELTA_MASK        0xF0
#define COAP_HEADER_OPTION_SHORT_LENGTH_MASK 0x0F

/*
 * Conservative size limit, as not all options have to be set at the same time.
 */
//TODO used for blocksize
#ifndef COAP_MAX_HEADER_SIZE
/*                            Hdr CoT Age  Tag              Obs  Tok               Blo strings */
#define COAP_MAX_HEADER_SIZE  (4 + 3 + 5 + 1+COAP_ETAG_LEN + 3 + 1+COAP_TOKEN_LEN + 4 + 30) /* 70 */
#endif /* COAP_MAX_HEADER_SIZE */

#define COAP_MAX_PACKET_SIZE  (COAP_MAX_HEADER_SIZE + REST_MAX_CHUNK_SIZE)
/*                                        0/14          48 for IPv6 (28 for IPv4) */
#if COAP_MAX_PACKET_SIZE > (UIP_BUFSIZE - UIP_LLH_LEN - UIP_IPUDPH_LEN)
//#error "UIP_CONF_BUFFER_SIZE too small for REST_MAX_CHUNK_SIZE"
#endif


/* Bitmap for set options */
enum { OPTION_MAP_SIZE = sizeof(uint8_t) * 8 };
#define SET_OPTION(packet, opt) ((packet)->options[opt / OPTION_MAP_SIZE] |= 1 << (opt % OPTION_MAP_SIZE))
#define IS_OPTION(packet, opt) ((packet)->options[opt / OPTION_MAP_SIZE] & (1 << (opt % OPTION_MAP_SIZE)))

#ifndef MIN
#define MIN(a, b) ((a) < (b)? (a) : (b))
#endif /* MIN */

/* CoAP message types */
typedef enum
{
  COAP_TYPE_CON, /* confirmables */
  COAP_TYPE_NON, /* non-confirmables */
  COAP_TYPE_ACK, /* acknowledgements */
  COAP_TYPE_RST  /* reset */
} coap_message_type_t;

/* CoAP request method codes */
typedef enum
{
  COAP_GET = 1,
  COAP_POST,
  COAP_PUT,
  COAP_DELETE
} coap_method_t;

/* CoAP response codes */
// typedef enum {
//   COAP_NO_ERROR = 0,
// 
//   CREATED_2_01 = 65,                    /* CREATED */
//   DELETED_2_02 = 66,                    /* DELETED */
//   VALID_2_03 = 67,                      /* NOT_MODIFIED */
//   COAP_204_CHANGED = 68,                    /* CHANGED */
//   COAP_205_CONTENT = 69,                    /* OK */
// 
//   COAP_400_BAD_REQUEST = 128,               /* BAD_REQUEST */
//   UNAUTHORIZED_4_01 = 129,              /* UNAUTHORIZED */
//   COAP_402_BAD_OPTION = 130,                /* BAD_OPTION */
//   FORBIDDEN_4_03 = 131,                 /* FORBIDDEN */
//   COAP_404_NOT_FOUND = 132,                 /* NOT_FOUND */
//   METHOD_NOT_ALLOWED_4_05 = 133,        /* METHOD_NOT_ALLOWED */
//   COAP_406_NOT_ACCEPTABLE = 134,            /* NOT_ACCEPTABLE */
//   PRECONDITION_FAILED_4_12 = 140,       /* BAD_REQUEST */
//   REQUEST_ENTITY_TOO_LARGE_4_13 = 141,  /* REQUEST_ENTITY_TOO_LARGE */
//   UNSUPPORTED_MEDIA_TYPE_4_15 = 143,    /* UNSUPPORTED_MEDIA_TYPE */
// 
//   COAP_500_INTERNAL_SERVER_ERROR = 160,     /* INTERNAL_SERVER_ERROR */
//   COAP_501_NOT_IMPLEMENTED = 161,           /* NOT_IMPLEMENTED */
//   BAD_GATEWAY_5_02 = 162,               /* BAD_GATEWAY */
//   SERVICE_UNAVAILABLE_5_03 = 163,       /* SERVICE_UNAVAILABLE */
//   GATEWAY_TIMEOUT_5_04 = 164,           /* GATEWAY_TIMEOUT */
//   PROXYING_NOT_SUPPORTED_5_05 = 165,    /* PROXYING_NOT_SUPPORTED */
// 
//   /* Erbium errors */
//   MEMORY_ALLOCATION_ERROR = 192,
//   PACKET_SERIALIZATION_ERROR,
// 
//   /* Erbium hooks */
//   MANUAL_RESPONSE
// 
// } coap_status_t_org;

/* CoAP header options */
typedef enum 
{
  COAP_OPTION_IF_MATCH = 1,       /* 0-8 B */
  COAP_OPTION_URI_HOST = 3,       /* 1-255 B */
  COAP_OPTION_ETAG = 4,           /* 1-8 B */
  COAP_OPTION_IF_NONE_MATCH = 5,  /* 0 B */
  COAP_OPTION_OBSERVE = 6,        /* 0-3 B */
  COAP_OPTION_URI_PORT = 7,       /* 0-2 B */
  COAP_OPTION_LOCATION_PATH = 8,  /* 0-255 B */
  COAP_OPTION_URI_PATH = 11,      /* 0-255 B */
  COAP_OPTION_CONTENT_TYPE = 12,  /* 0-2 B */
  COAP_OPTION_MAX_AGE = 14,       /* 0-4 B */
  COAP_OPTION_URI_QUERY = 15,     /* 0-270 B */
  COAP_OPTION_ACCEPT = 17,        /* 0-2 B */
  COAP_OPTION_TOKEN = 19,         /* 1-8 B */
  COAP_OPTION_LOCATION_QUERY = 20, /* 1-270 B */
  COAP_OPTION_BLOCK2 = 23,        /* 1-3 B */
  COAP_OPTION_BLOCK1 = 27,        /* 1-3 B */
  COAP_OPTION_SIZE = 28,          /* 0-4 B */
  COAP_OPTION_PROXY_URI = 35,     /* 1-270 B */
  COAP_OPTION_AUTH_CODE = 63,  /* 1-270 B OneNET defined option, use for authentication */
} coap_option_t;

/* CoAP Content-Types */
typedef enum 
{
  TEXT_PLAIN = 0,
  TEXT_XML = 1, /* Indented types are not in the initial registry. */
  TEXT_CSV = 2,
  TEXT_HTML = 3,
  IMAGE_GIF = 21,
  IMAGE_JPEG = 22,
  IMAGE_PNG = 23,
  IMAGE_TIFF = 24,
  AUDIO_RAW = 25,
  VIDEO_RAW = 26,
  APPLICATION_LINK_FORMAT = 40,
  APPLICATION_XML = 41,
  APPLICATION_OCTET_STREAM = 42,
  APPLICATION_RDF_XML = 43,
  APPLICATION_SOAP_XML = 44,
  APPLICATION_ATOM_XML = 45,
  APPLICATION_XMPP_XML = 46,
  APPLICATION_EXI = 47,
  APPLICATION_FASTINFOSET = 48,
  APPLICATION_SOAP_FASTINFOSET = 49,
  APPLICATION_JSON = 50,
  APPLICATION_X_OBIX_BINARY = 51,

  CONTENT_TYPE_MAX = 0xFFFF
} coap_content_type_t;

typedef struct _multi_option_t 
{
  struct _multi_option_t *next;
  uint8_t is_static;
  uint8_t len;
  uint8_t *data;
} multi_option_t;

/* Parsed message struct */
// ADD FOR ATCMD
typedef struct _coap_packet_t{
  struct _coap_packet_t *next;
  uint8_t *buffer; /* pointer to CoAP header / incoming packet buffer / memory to serialize packet */

  uint8_t version;
  coap_message_type_t type;
  uint8_t code;
  uint16_t mid;
  uint8_t             options[COAP_OPTION_AUTH_CODE / OPTION_MAP_SIZE + 1]; /* Bitmap to check if option is set */
  coap_content_type_t content_type; /* Parse options once and store; allows setting options in random order  */
  uint32_t max_age;
  size_t proxy_uri_len;
  const uint8_t *proxy_uri;
   size_t              auth_code_len;
   const uint8_t      *auth_code;
  uint8_t etag_len;
  uint8_t etag[COAP_ETAG_LEN];
  size_t uri_host_len;
  const uint8_t *uri_host;
  multi_option_t *location_path;
  uint16_t uri_port;
  size_t location_query_len;
  uint8_t *location_query;
  multi_option_t *uri_path;
  uint32_t observe;
  uint8_t token_len;
  uint8_t token[COAP_TOKEN_LEN];
  uint8_t accept_num;
  uint16_t accept[COAP_MAX_ACCEPT_NUM];
  uint8_t if_match_len;
  uint8_t if_match[COAP_ETAG_LEN];
  uint32_t block2_num;
  uint8_t block2_more;
  uint16_t block2_size;
  uint32_t block2_offset;
  uint32_t block1_num;
  uint8_t block1_more;
  uint16_t block1_size;
  uint32_t block1_offset;
  uint32_t size;
  multi_option_t *uri_query;
  uint8_t if_none_match;

  //ADD FOR ATCMD
  uint8_t format;

  uint16_t payload_len;
  uint8_t *payload;

} coap_packet_t;

/* Option format serialization*/
#define COAP_SERIALIZE_INT_OPTION(number, field, text)  \
    if (IS_OPTION(coap_pkt, number)) { \
      PRINTF(text" [%u]\n", coap_pkt->field); \
      option += coap_serialize_int_option(number, current_number, option, coap_pkt->field); \
      current_number = number; \
    }
#define COAP_SERIALIZE_BYTE_OPTION(number, field, text)      \
    if (IS_OPTION(coap_pkt, number)) { \
      PRINTF(text" %u [0x%02X%02X%02X%02X%02X%02X%02X%02X]\n", coap_pkt->field##_len, \
        coap_pkt->field[0], \
        coap_pkt->field[1], \
        coap_pkt->field[2], \
        coap_pkt->field[3], \
        coap_pkt->field[4], \
        coap_pkt->field[5], \
        coap_pkt->field[6], \
        coap_pkt->field[7] \
      ); /*FIXME always prints 8 bytes */ \
      option += coap_serialize_array_option(number, current_number, option, coap_pkt->field, coap_pkt->field##_len, '\0'); \
      current_number = number; \
    }
#define COAP_SERIALIZE_STRING_OPTION(number, field, splitter, text)      \
    if (IS_OPTION(coap_pkt, number)) { \
      PRINTF(text" [%.*s]\n", coap_pkt->field##_len, coap_pkt->field); \
      option += coap_serialize_array_option(number, current_number, option, (uint8_t *) coap_pkt->field, coap_pkt->field##_len, splitter); \
      current_number = number; \
    }
#define COAP_SERIALIZE_MULTI_OPTION(number, field, text)      \
        if (IS_OPTION(coap_pkt, number)) { \
          PRINTF(text); \
          PRINTF("\n"); \
          option += coap_serialize_multi_option(number, current_number, option, coap_pkt->field); \
          current_number = number; \
        }
#define COAP_SERIALIZE_ACCEPT_OPTION(number, field, text)  \
    if (IS_OPTION(coap_pkt, number)) { \
      int i; \
      for (i=0; i<coap_pkt->field##_num; ++i) \
      { \
        PRINTF(text" [%u]\n", coap_pkt->field[i]); \
        option += coap_serialize_int_option(number, current_number, option, coap_pkt->field[i]); \
        current_number = number; \
      } \
    }
#define COAP_SERIALIZE_BLOCK_OPTION(number, field, text)      \
    if (IS_OPTION(coap_pkt, number)) \
    { \
      PRINTF(text" [%lu%s (%u B/blk)]\n", coap_pkt->field##_num, coap_pkt->field##_more ? "+" : "", coap_pkt->field##_size); \
      uint32_t block = coap_pkt->field##_num << 4; \
      if (coap_pkt->field##_more) block |= 0x8; \
      block |= 0xF & coap_log_2(coap_pkt->field##_size/16); \
      PRINTF(text" encoded: 0x%lX\n", block); \
      option += coap_serialize_int_option(number, current_number, option, block); \
      current_number = number; \
    }

/* To store error code and human-readable payload */
extern const char *coap_error_message;

uint16_t coap_get_mid(void);

void    coap_init_message(void *packet, coap_message_type_t type, uint8_t code, uint16_t mid);
size_t  coap_serialize_get_size(void *packet);
size_t  coap_serialize_message(void *packet, uint8_t *buffer);
coap_status_t coap_parse_message(void *request, uint8_t *data, uint16_t data_len);
void    coap_free_header(void *packet);

char *  coap_get_multi_option_as_string(multi_option_t * option);
void    coap_add_multi_option(multi_option_t **dst, uint8_t *option, size_t option_len, uint8_t is_static);
void    free_multi_option(multi_option_t *dst);

/*-----------------------------------------------------------------------------------*/

int coap_set_status_code(void *packet, unsigned int code);

int coap_get_header_content_type(void *packet);
int coap_set_header_content_type(void *packet, unsigned int content_type);

int coap_get_header_accept(void *packet, const uint16_t **accept);
int coap_set_header_accept(void *packet, uint16_t accept);

int coap_get_header_max_age(void *packet, uint32_t *age);
int coap_set_header_max_age(void *packet, uint32_t age);

int coap_get_header_etag(void *packet, const uint8_t **etag);
int coap_set_header_etag(void *packet, const uint8_t *etag, size_t etag_len);

int coap_get_header_if_match(void *packet, const uint8_t **etag);
int coap_set_header_if_match(void *packet, const uint8_t *etag, size_t etag_len);

int coap_get_header_if_none_match(void *packet);
int coap_set_header_if_none_match(void *packet);

int coap_get_header_token(void *packet, const uint8_t **token);
int coap_set_header_token(void *packet, const uint8_t *token, size_t token_len);

int coap_get_header_proxy_uri(void *packet, const char **uri); /* In-place string might not be 0-terminated. */
int coap_set_header_proxy_uri(void *packet, const char *uri);

int coap_get_header_uri_host(void *packet, const char **host); /* In-place string might not be 0-terminated. */
int coap_set_header_uri_host(void *packet, const char *host);

int coap_get_header_uri_path(void *packet, const char **path); /* In-place string might not be 0-terminated. */
int coap_set_header_uri_path(void *packet, const char *path);
int coap_set_header_uri_path_segment(void *packet, const char *path);

int coap_get_header_uri_query(void *packet, const char **query); /* In-place string might not be 0-terminated. */
int coap_set_header_uri_query(void *packet, const char *query);

int coap_get_header_location_path(void *packet, const char **path); /* In-place string might not be 0-terminated. */
int coap_set_header_location_path(void *packet, const char *path); /* Also splits optional query into Location-Query option. */

int coap_get_header_location_query(void *packet, const char **query); /* In-place string might not be 0-terminated. */
int coap_set_header_location_query(void *packet, char *query);

int coap_get_header_observe(void *packet, uint32_t *observe);
int coap_set_header_observe(void *packet, uint32_t observe);

int coap_get_header_block2(void *packet, uint32_t *num, uint8_t *more, uint16_t *size, uint32_t *offset);
int coap_set_header_block2(void *packet, uint32_t num, uint8_t more, uint16_t size);

int coap_get_header_block1(void *packet, uint32_t *num, uint8_t *more, uint16_t *size, uint32_t *offset);
int coap_set_header_block1(void *packet, uint32_t num, uint8_t more, uint16_t size);

int coap_get_header_size(void *packet, uint32_t *size);
int coap_set_header_size(void *packet, uint32_t size);

int coap_get_payload(void *packet, const uint8_t **payload);
int coap_set_payload(void *packet, const void *payload, size_t length);

int coap_set_header_auth_code( void *packet,const char *code );
#endif /* COAP_13_H_ */
