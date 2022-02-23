/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include "infra_config.h"
#include "infra_compat.h"
#include "mbedtls/error.h"
#include "mbedtls/ssl.h"
#include "mbedtls/net.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/pk.h"
#include "mbedtls/debug.h"
#include "mbedtls/platform.h"
#include "wrappers.h"
#include "infra_sha256.h"
#include "infra_string.h"
#include <errno.h>


#if MBEDTLS_SSL_MAX_CONTENT_LEN < 4096
#error "MBEDTLS_SSL_MAX_CONTENT_LEN need more than 4096 bytes if use tls in ali-iot-kit"
#endif


#define SEND_TIMEOUT_SECONDS                (10)

#ifndef CONFIG_MBEDTLS_DEBUG_LEVEL
    #define CONFIG_MBEDTLS_DEBUG_LEVEL 0
#endif

#define DBG_EXT_TAG                        "ali.tls"
#define DBG_EXT_LVL                        DBG_EXT_INFO
#include "dlog.h"


typedef struct _TLSDataParams {
    mbedtls_ssl_context ssl;          /**< mbed TLS control context. */
    mbedtls_net_context fd;           /**< mbed TLS network context. */
    mbedtls_ssl_config conf;          /**< mbed TLS configuration context. */
    mbedtls_x509_crt cacertl;         /**< mbed TLS CA certification. */
    mbedtls_x509_crt clicert;         /**< mbed TLS Client certification. */
    mbedtls_pk_context pkey;          /**< mbed TLS Client key. */
} TLSDataParams_t, *TLSDataParams_pt;

static unsigned int mbedtls_mem_used = 0;
static unsigned int mbedtls_max_mem_used = 0;
static ssl_hooks_t g_ssl_hooks = {HAL_Malloc, HAL_Free};

#define MBEDTLS_MEM_INFO_MAGIC   0x12345678

typedef struct {
    int magic;
    int size;
} mbedtls_mem_info_t;


#if defined(TLS_SAVE_TICKET)

#define KEY_MAX_LEN          64
#define TLS_MAX_SESSION_BUF  4096
#define KV_SESSION_KEY_FMT   "TLS_%s"

extern int HAL_Kv_Set(const char *key, const void *val, int len, int sync);

extern int HAL_Kv_Get(const char *key, void *val, int *buffer_len);

static mbedtls_ssl_session *saved_session = NULL;

static int ssl_serialize_session(const mbedtls_ssl_session *session,
                                 unsigned char *buf, size_t buf_len,
                                 size_t *olen)
{
    unsigned char *p = buf;
    size_t left = buf_len;

    if (left < sizeof(mbedtls_ssl_session)) {
        return (MBEDTLS_ERR_SSL_BUFFER_TOO_SMALL);
    }

    memcpy(p, session, sizeof(mbedtls_ssl_session));
    p += sizeof(mbedtls_ssl_session);
    left -= sizeof(mbedtls_ssl_session);
#if defined(MBEDTLS_SSL_SESSION_TICKETS) && defined(MBEDTLS_SSL_CLI_C)
    if (left < sizeof(mbedtls_ssl_session)) {
        return (MBEDTLS_ERR_SSL_BUFFER_TOO_SMALL);
    }
    memcpy(p, session->ticket, session->ticket_len);
    p += session->ticket_len;
    left -= session->ticket_len;
#endif

    *olen = p - buf;

    return (0);
}

static int ssl_deserialize_session(mbedtls_ssl_session *session,
                                   const unsigned char *buf, size_t len)
{
    const unsigned char *p = buf;
    const unsigned char *const end = buf + len;

    if (sizeof(mbedtls_ssl_session) > (size_t)(end - p)) {
        return (MBEDTLS_ERR_SSL_BAD_INPUT_DATA);
    }

    memcpy(session, p, sizeof(mbedtls_ssl_session));
    p += sizeof(mbedtls_ssl_session);
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    session->peer_cert = NULL;
#endif

#if defined(MBEDTLS_SSL_SESSION_TICKETS) && defined(MBEDTLS_SSL_CLI_C)
    if (session->ticket_len > 0) {
        if (session->ticket_len > (size_t)(end - p)) {
            return (MBEDTLS_ERR_SSL_BAD_INPUT_DATA);
        }
        session->ticket = HAL_Malloc(session->ticket_len);
        if (session->ticket == NULL) {
            return (MBEDTLS_ERR_SSL_ALLOC_FAILED);
        }
        memcpy(session->ticket, p, session->ticket_len);
        p += session->ticket_len;
        LOG_I(DBG_EXT_TAG, "saved ticket len = %d ", (int)session->ticket_len);
    }
#endif

    if (p != end) {
        return (MBEDTLS_ERR_SSL_BAD_INPUT_DATA);
    }

    return (0);
}
#endif

static void _srand(unsigned int seed)
{
#ifdef HAL_KV
#define SEED_MAGIC 0x123
    int           ret        = 0;
    int           seed_len   = 0;
    unsigned int  seed_val   = 0;
    static char  *g_seed_key = "seed_key";

    seed_len = sizeof(seed_val);
    ret = HAL_Kv_Get(g_seed_key, &seed_val, &seed_len);
    if (ret) {
        seed_val = SEED_MAGIC;
    }
    seed_val += seed;
    HAL_Srandom(seed_val);
    seed_val = rand();
    HAL_Kv_Set(g_seed_key, &seed_val, sizeof(seed_val), 1);
#else
    HAL_Srandom(seed);
#endif
}


static unsigned int _avRandom()
{
    return (((unsigned int)rand() << 16) + rand());
}

static int _ssl_random(void *p_rng, unsigned char *output, size_t output_len)
{
    uint32_t rnglen = output_len;
    uint8_t   rngoffset = 0;

    while (rnglen > 0) {
        *(output + rngoffset) = (unsigned char)_avRandom() ;
        rngoffset++;
        rnglen--;
    }
    return 0;
}

static void _ssl_debug(void *ctx, int level, const char *file, int line, const char *str)
{
    ((void) level);
    if (NULL != ctx) {
#if 0
        fprintf((FILE *) ctx, "%s:%04d: %s", file, line, str);
        fflush((FILE *) ctx);
#endif
        LOG_I(DBG_EXT_TAG, "%s", str);
    }
}

static int _real_confirm(int verify_result)
{
    LOG_I(DBG_EXT_TAG, "certificate verification result: 0x%02x", verify_result);

#if defined(FORCE_SSL_VERIFY)
    if ((verify_result & MBEDTLS_X509_BADCERT_EXPIRED) != 0) {
        LOG_E(DBG_EXT_TAG, "! fail ! ERROR_CERTIFICATE_EXPIRED");
        return -1;
    }

    if ((verify_result & MBEDTLS_X509_BADCERT_REVOKED) != 0) {
        LOG_E(DBG_EXT_TAG, "! fail ! server certificate has been revoked");
        return -1;
    }

    if ((verify_result & MBEDTLS_X509_BADCERT_CN_MISMATCH) != 0) {
        LOG_E(DBG_EXT_TAG, "! fail ! CN mismatch");
        return -1;
    }

    if ((verify_result & MBEDTLS_X509_BADCERT_NOT_TRUSTED) != 0) {
        LOG_E(DBG_EXT_TAG, "! fail ! self-signed or not signed by a trusted CA");
        return -1;
    }
#endif

    return 0;
}

static int _ssl_client_init(mbedtls_ssl_context *ssl,
                            mbedtls_net_context *tcp_fd,
                            mbedtls_ssl_config *conf,
                            mbedtls_x509_crt *crt509_ca, const char *ca_crt, size_t ca_len,
                            mbedtls_x509_crt *crt509_cli, const char *cli_crt, size_t cli_len,
                            mbedtls_pk_context *pk_cli, const char *cli_key, size_t key_len,  const char *cli_pwd, size_t pwd_len
                           )
{
    int ret = -1;

    /*
     * 0. Initialize the RNG and the session data
     */
#if defined(MBEDTLS_DEBUG_C)
    mbedtls_debug_set_threshold((int)CONFIG_MBEDTLS_DEBUG_LEVEL);
#endif
    mbedtls_net_init(tcp_fd);
    mbedtls_ssl_init(ssl);
    mbedtls_ssl_config_init(conf);

    _srand(HAL_UptimeMs());
    /* Setup Client Cert/Key */
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    mbedtls_x509_crt_init(crt509_ca);

    /*verify_source->trusted_ca_crt==NULL
     * 0. Initialize certificates
     */

    LOG_I(DBG_EXT_TAG, "Loading the CA root certificate ...");
    if (NULL != ca_crt) {
        if (0 != (ret = mbedtls_x509_crt_parse(crt509_ca, (const unsigned char *)ca_crt, ca_len))) {
            LOG_E(DBG_EXT_TAG, " failed ! x509parse_crt returned -0x%04x", -ret);
            return ret;
        }
    }
    LOG_I(DBG_EXT_TAG, " ok (%d skipped)", ret);

#if defined(MBEDTLS_CERTS_C)
    mbedtls_x509_crt_init(crt509_cli);
    mbedtls_pk_init(pk_cli);
#endif
    if (cli_crt != NULL && cli_key != NULL) {
#if defined(MBEDTLS_CERTS_C)
        LOG_I(DBG_EXT_TAG, "start prepare client cert .");
        ret = mbedtls_x509_crt_parse(crt509_cli, (const unsigned char *) cli_crt, cli_len);
#else
        {
            ret = 1;
            LOG_E(DBG_EXT_TAG, "MBEDTLS_CERTS_C not defined.");
        }
#endif
        if (ret != 0) {
            LOG_E(DBG_EXT_TAG, " failed!  mbedtls_x509_crt_parse returned -0x%x", -ret);
            return ret;
        }

#if defined(MBEDTLS_CERTS_C)
        LOG_I(DBG_EXT_TAG, "start mbedtls_pk_parse_key[%s]", cli_pwd);
        ret = mbedtls_pk_parse_key(pk_cli, (const unsigned char *) cli_key, key_len, (const unsigned char *) cli_pwd, pwd_len);
#else
        {
            ret = 1;
            LOG_E(DBG_EXT_TAG, "MBEDTLS_CERTS_C not defined.");
        }
#endif

        if (ret != 0) {
            LOG_E(DBG_EXT_TAG, " failed\n  !  mbedtls_pk_parse_key returned -0x%x", -ret);
            return ret;
        }
    }
#endif /* MBEDTLS_X509_CRT_PARSE_C */

    return 0;
}


void *_SSLCalloc_wrapper(size_t n, size_t size)
{
    unsigned char *buf = NULL;
    mbedtls_mem_info_t *mem_info = NULL;

    if (n == 0 || size == 0) {
        return NULL;
    }

    buf = (unsigned char *)(g_ssl_hooks.malloc(n * size + sizeof(mbedtls_mem_info_t)));
    if (NULL == buf) {
        return NULL;
    } else {
        memset(buf, 0, n * size + sizeof(mbedtls_mem_info_t));
    }

    mem_info = (mbedtls_mem_info_t *)buf;
    mem_info->magic = MBEDTLS_MEM_INFO_MAGIC;
    mem_info->size = n * size;
    buf += sizeof(mbedtls_mem_info_t);

    mbedtls_mem_used += mem_info->size;
    if (mbedtls_mem_used > mbedtls_max_mem_used) {
        mbedtls_max_mem_used = mbedtls_mem_used;
    }

    /* LOG_I("INFO -- mbedtls malloc: %p %d  total used: %d  max used: %d\r\n",
                       buf, (int)size, mbedtls_mem_used, mbedtls_max_mem_used); */

    return buf;
}

void _SSLFree_wrapper(void *ptr)
{
    mbedtls_mem_info_t *mem_info = NULL;
    if (NULL == ptr) {
        return;
    }

    mem_info = (mbedtls_mem_info_t *)((unsigned char *)ptr - sizeof(mbedtls_mem_info_t));
    if (mem_info->magic != MBEDTLS_MEM_INFO_MAGIC) {
        LOG_W(DBG_EXT_TAG, "Warning - invalid mem info magic: 0x%x", mem_info->magic);
        return;
    }

    mbedtls_mem_used -= mem_info->size;
    /* LOG_E("INFO mbedtls free: %p %d  total used: %d  max used: %d\r\n",
                       ptr, mem_info->size, mbedtls_mem_used, mbedtls_max_mem_used);*/

    g_ssl_hooks.free(mem_info);
}

/**
 * @brief This function connects to the specific SSL server with TLS, and returns a value that indicates whether the connection is create successfully or not. Call #NewNetwork() to initialize network structure before calling this function.
 * @param[in] n is the the network structure pointer.
 * @param[in] addr is the Server Host name or IP address.
 * @param[in] port is the Server Port.
 * @param[in] ca_crt is the Server's CA certification.
 * @param[in] ca_crt_len is the length of Server's CA certification.
 * @param[in] client_crt is the client certification.
 * @param[in] client_crt_len is the length of client certification.
 * @param[in] client_key is the client key.
 * @param[in] client_key_len is the length of client key.
 * @param[in] client_pwd is the password of client key.
 * @param[in] client_pwd_len is the length of client key's password.
 * @sa #NewNetwork();
 * @return If the return value is 0, the connection is created successfully. If the return value is -1, then calling lwIP #socket() has failed. If the return value is -2, then calling lwIP #connect() has failed. Any other value indicates that calling lwIP #getaddrinfo() has failed.
 */

static int _TLSConnectNetwork(TLSDataParams_t *pTlsData, const char *addr, const char *port,
                              const char *ca_crt, size_t ca_crt_len,
                              const char *client_crt,   size_t client_crt_len,
                              const char *client_key,   size_t client_key_len,
                              const char *client_pwd, size_t client_pwd_len)
{
    int ret = -1;
    /*
     * 0. Init
     */
    if (0 != (ret = _ssl_client_init(&(pTlsData->ssl), &(pTlsData->fd), &(pTlsData->conf),
                                     &(pTlsData->cacertl), ca_crt, ca_crt_len,
                                     &(pTlsData->clicert), client_crt, client_crt_len,
                                     &(pTlsData->pkey), client_key, client_key_len, client_pwd, client_pwd_len))) {
        LOG_E(DBG_EXT_TAG, " failed ! ssl_client_init returned -0x%04x", -ret);
        return ret;
    }

    /*
     * 1. Start the connection
     */
    LOG_I(DBG_EXT_TAG, "Connecting to /%s/%s...", addr, port);

    if (0 != (ret = mbedtls_net_connect(&(pTlsData->fd), addr, port, MBEDTLS_NET_PROTO_TCP))) {
        pTlsData->fd.fd = -1;
        LOG_E(DBG_EXT_TAG, " failed ! net_connect returned -0x%04x", -ret);
        return ret;
    }

    LOG_I(DBG_EXT_TAG, " ok");

    /*
     * 2. Setup stuff
     */
    LOG_I(DBG_EXT_TAG, "  . Setting up the SSL/TLS structure...");
    if ((ret = mbedtls_ssl_config_defaults(&(pTlsData->conf), MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM,
                                           MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
        LOG_E(DBG_EXT_TAG, " failed! mbedtls_ssl_config_defaults returned %d", ret);
        return ret;
    }

    mbedtls_ssl_conf_max_version(&pTlsData->conf, MBEDTLS_SSL_MAJOR_VERSION_3, MBEDTLS_SSL_MINOR_VERSION_3);
    mbedtls_ssl_conf_min_version(&pTlsData->conf, MBEDTLS_SSL_MAJOR_VERSION_3, MBEDTLS_SSL_MINOR_VERSION_3);

    LOG_I(DBG_EXT_TAG, " ok");

    /* OPTIONAL is not optimal for security, but makes interop easier in this simplified example */
    if (ca_crt != NULL) {
#if defined(FORCE_SSL_VERIFY)
        mbedtls_ssl_conf_authmode(&(pTlsData->conf), MBEDTLS_SSL_VERIFY_REQUIRED);
#else
        mbedtls_ssl_conf_authmode(&(pTlsData->conf), MBEDTLS_SSL_VERIFY_OPTIONAL);
#endif
    } else {
        mbedtls_ssl_conf_authmode(&(pTlsData->conf), MBEDTLS_SSL_VERIFY_NONE);
    }

#if defined(MBEDTLS_X509_CRT_PARSE_C)
    mbedtls_ssl_conf_ca_chain(&(pTlsData->conf), &(pTlsData->cacertl), NULL);

    if ((ret = mbedtls_ssl_conf_own_cert(&(pTlsData->conf), &(pTlsData->clicert), &(pTlsData->pkey))) != 0) {
        LOG_E(DBG_EXT_TAG, " failed\n  ! mbedtls_ssl_conf_own_cert returned %d", ret);
        return ret;
    }
#endif
    mbedtls_ssl_conf_rng(&(pTlsData->conf), _ssl_random, NULL);
    mbedtls_ssl_conf_dbg(&(pTlsData->conf), _ssl_debug, NULL);
    mbedtls_ssl_conf_dbg(&(pTlsData->conf), _ssl_debug, stdout);

    if ((ret = mbedtls_ssl_setup(&(pTlsData->ssl), &(pTlsData->conf))) != 0) {
        LOG_E(DBG_EXT_TAG, "failed! mbedtls_ssl_setup returned %d", ret);
        return ret;
    }
    mbedtls_ssl_set_hostname(&(pTlsData->ssl), addr);
    mbedtls_ssl_set_bio(&(pTlsData->ssl), &(pTlsData->fd), mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout);

#if defined(TLS_SAVE_TICKET)
    if (NULL == saved_session) {
        do {
            int len = TLS_MAX_SESSION_BUF;
            char key_buf[KEY_MAX_LEN] = {0};
            unsigned char *save_buf = HAL_Malloc(TLS_MAX_SESSION_BUF);
            if (save_buf ==  NULL) {
                LOG_E(DBG_EXT_TAG, " malloc failed");
                break;
            }

            saved_session = HAL_Malloc(sizeof(mbedtls_ssl_session));

            if (saved_session == NULL) {
                LOG_E(DBG_EXT_TAG, " malloc failed");
                HAL_Free(save_buf);
                save_buf =  NULL;
                break;
            }


            memset(save_buf, 0x00, TLS_MAX_SESSION_BUF);
            memset(saved_session, 0x00, sizeof(mbedtls_ssl_session));

            HAL_Snprintf(key_buf,KEY_MAX_LEN -1, KV_SESSION_KEY_FMT, addr);
            ret = HAL_Kv_Get(key_buf, save_buf, &len);

            if (ret != 0 || len == 0) {
                LOG_E(DBG_EXT_TAG, " kv get failed len=%d,ret = %d", len, ret);
                HAL_Free(saved_session);
                HAL_Free(save_buf);
                save_buf = NULL;
                saved_session = NULL;
                break;
            }
            ret = ssl_deserialize_session(saved_session, save_buf, len);
            if (ret < 0) {
                LOG_E(DBG_EXT_TAG, "ssl_deserialize_session err,ret = 0x%x", ret);
                HAL_Free(saved_session);
                HAL_Free(save_buf);
                save_buf = NULL;
                saved_session = NULL;
                break;
            }
            HAL_Free(save_buf);
        } while (0);
    }

    if (NULL != saved_session) {
        mbedtls_ssl_set_session(&(pTlsData->ssl), saved_session);
        LOG_I(DBG_EXT_TAG, "use saved session!!");
    }
#endif
    /*
      * 4. Handshake
      */
    mbedtls_ssl_conf_read_timeout(&(pTlsData->conf), 10000);
    LOG_I(DBG_EXT_TAG, "Performing the SSL/TLS handshake...");

    while ((ret = mbedtls_ssl_handshake(&(pTlsData->ssl))) != 0) {
        if ((ret != MBEDTLS_ERR_SSL_WANT_READ) && (ret != MBEDTLS_ERR_SSL_WANT_WRITE)) {
            LOG_E(DBG_EXT_TAG, "failed  ! mbedtls_ssl_handshake returned -0x%04x", -ret);
#if defined(TLS_SAVE_TICKET)
            if (saved_session != NULL) {
                mbedtls_ssl_session_free(saved_session);
                HAL_Free(saved_session);
                saved_session = NULL;
            }
#endif
            return ret;
        }
    }
    LOG_I(DBG_EXT_TAG, " ok");

#if defined(TLS_SAVE_TICKET)
    do {
        size_t real_session_len = 0;
        mbedtls_ssl_session *new_session = NULL;

        new_session = HAL_Malloc(sizeof(mbedtls_ssl_session));
        if (NULL == new_session) {
            break;
        }

        memset(new_session, 0x00, sizeof(mbedtls_ssl_session));

        ret = mbedtls_ssl_get_session(&(pTlsData->ssl), new_session);
        if (ret != 0) {
            HAL_Free(new_session);
            break;
        }
        if (saved_session == NULL) {
            ret = 1;
        } else if (new_session->ticket_len != saved_session->ticket_len) {
            ret = 1;
        } else {
            ret = memcmp(new_session->ticket, saved_session->ticket, new_session->ticket_len);
        }
        if (ret != 0) {
            unsigned char *save_buf = HAL_Malloc(TLS_MAX_SESSION_BUF);
            if (save_buf ==  NULL) {
                mbedtls_ssl_session_free(new_session);
                HAL_Free(new_session);
                new_session = NULL;
                break;
            }
            memset(save_buf, 0x00, sizeof(TLS_MAX_SESSION_BUF));
            ret = ssl_serialize_session(new_session, save_buf, TLS_MAX_SESSION_BUF, &real_session_len);
            LOG_I(DBG_EXT_TAG, "mbedtls_ssl_get_session_session return 0x%04x real_len=%d", ret, (int)real_session_len);
            if (ret == 0) {
                char key_buf[KEY_MAX_LEN] = {0};
                HAL_Snprintf(key_buf,KEY_MAX_LEN -1, KV_SESSION_KEY_FMT, addr);
                ret = HAL_Kv_Set(key_buf, (void *)save_buf, real_session_len, 1);
                if (ret < 0) {
                    LOG_E(DBG_EXT_TAG, "save ticket to kv failed ret =%d ,len = %d", ret, (int)real_session_len);
                }
            }
            HAL_Free(save_buf);
        }
        mbedtls_ssl_session_free(new_session);
        HAL_Free(new_session);
    } while (0);
    if (saved_session != NULL) {
        mbedtls_ssl_session_free(saved_session);
        HAL_Free(saved_session);
        saved_session = NULL;
    }
#endif

    /*
     * 5. Verify the server certificate
     */
    LOG_I(DBG_EXT_TAG, "  . Verifying peer X.509 certificate..");
    if (0 != (ret = _real_confirm(mbedtls_ssl_get_verify_result(&(pTlsData->ssl))))) {
        LOG_E(DBG_EXT_TAG, " failed  ! verify result not confirmed.");
        return ret;
    }

    /* n->my_socket = (int)((n->tlsdataparams.fd).fd); */
    /* WRITE_IOT_DEBUG_LOG("my_socket=%d", n->my_socket); */

    return 0;

}

static int _network_ssl_read(TLSDataParams_t *pTlsData, char *buffer, int len, int timeout_ms)
{
    uint32_t        readLen = 0;
    static int      net_status = 0;
    int             ret = -1;
    char            err_str[33];

    mbedtls_ssl_conf_read_timeout(&(pTlsData->conf), timeout_ms);
    while (readLen < len) {
        ret = mbedtls_ssl_read(&(pTlsData->ssl), (unsigned char *)(buffer + readLen), (len - readLen));
        if (ret > 0) {
            readLen += ret;
            net_status = 0;
        } else if (ret == 0) {
            /* if ret is 0 and net_status is -2, indicate the connection is closed during last call */
            return (net_status == -2) ? net_status : readLen;
        } else {
            if (MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY == ret) {
                mbedtls_strerror(ret, err_str, sizeof(err_str));
                LOG_E(DBG_EXT_TAG, "ssl recv error: code = %d, err_str = '%s'", ret, err_str);
                net_status = -2; /* connection is closed */
                break;
            } else if ((MBEDTLS_ERR_SSL_TIMEOUT == ret)
                       || (MBEDTLS_ERR_SSL_CONN_EOF == ret)
                       || (MBEDTLS_ERR_SSL_SESSION_TICKET_EXPIRED == ret)
                       || (MBEDTLS_ERR_SSL_NON_FATAL == ret)) {
                /* read already complete */
                /* if call mbedtls_ssl_read again, it will return 0 (means EOF) */

                return readLen;
            } else {
                mbedtls_strerror(ret, err_str, sizeof(err_str));
                LOG_E(DBG_EXT_TAG, "ssl recv error: code = %d, err_str = '%s'", ret, err_str);
                net_status = -1;
                return -1; /* Connection error */
            }
        }
    }

    return (readLen > 0) ? readLen : net_status;
}

static int _network_ssl_write(TLSDataParams_t *pTlsData, const char *buffer, int len, int timeout_ms)
{
    uint32_t writtenLen = 0;
    int ret = -1;

    if (pTlsData == NULL) {
        return -1;
    }

    while (writtenLen < len) {
        ret = mbedtls_ssl_write(&(pTlsData->ssl), (unsigned char *)(buffer + writtenLen), (len - writtenLen));
        if (ret > 0) {
            writtenLen += ret;
            continue;
        } else if (ret == 0) {
            LOG_E(DBG_EXT_TAG, "ssl write timeout");
            return 0;
        } else {
            char err_str[33];
            mbedtls_strerror(ret, err_str, sizeof(err_str));
            LOG_E(DBG_EXT_TAG, "ssl write fail, code=%d, str=%s", ret, err_str);
            return -1; /* Connnection error */
        }
    }

    return writtenLen;
}

static void _network_ssl_disconnect(TLSDataParams_t *pTlsData)
{
    mbedtls_ssl_close_notify(&(pTlsData->ssl));

    if((pTlsData->fd.fd) >= 0)
    {
        mbedtls_net_free(&(pTlsData->fd));
    }
    
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    mbedtls_x509_crt_free(&(pTlsData->cacertl));
    if ((pTlsData->pkey).pk_info != NULL) {
        LOG_I(DBG_EXT_TAG, "need release client crt&key");
#if defined(MBEDTLS_CERTS_C)
        mbedtls_x509_crt_free(&(pTlsData->clicert));
        mbedtls_pk_free(&(pTlsData->pkey));
#endif
    }
#endif
    mbedtls_ssl_free(&(pTlsData->ssl));
    mbedtls_ssl_config_free(&(pTlsData->conf));
    LOG_I(DBG_EXT_TAG, "ssl_disconnect");
}


int ssl_hooks_set(ssl_hooks_t *hooks)
{
    if (hooks == NULL || hooks->malloc == NULL || hooks->free == NULL) {
        return -1;
    }

    g_ssl_hooks.malloc = hooks->malloc;
    g_ssl_hooks.free = hooks->free;

    return 0;
}
const char *g_cli_crt = \
{
    \
//    "-----BEGIN CERTIFICATE-----\r\n"
//    "MIIDhzCCAm+gAwIBAgIHPSU4OIbUQDANBgkqhkiG9w0BAQsFADBTMSgwJgYDVQQD\r\n" \
//    "DB9BbGliYWJhIENsb3VkIElvVCBPcGVyYXRpb24gQ0ExMRowGAYDVQQKDBFBbGli\r\n" \
//    "YWJhIENsb3VkIElvVDELMAkGA1UEBhMCQ04wIBcNMTkxMTE0MDgwNTI5WhgPMjEx\r\n" \
//    "OTExMTQwODA1MjlaMFExJjAkBgNVBAMMHUFsaWJhYmEgQ2xvdWQgSW9UIENlcnRp\r\n" \
//    "ZmljYXRlMRowGAYDVQQKDBFBbGliYWJhIENsb3VkIElvVDELMAkGA1UEBhMCQ04w\r\n" \
//    "ggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCR8UK3ewlXW1R05wLdzL7U\r\n" \
//    "kGAMiA7s6EVFNqX98n1uPH8xMUq7AGyShtqn1X3yNDgJ0Uf49RthntIWE8FmRWvn\r\n" \
//    "I+LTrbWl8nCnhDjoKs2GezGKHu8ZtJFHc3Lb50LqkvUIkIGSbBKN4aSroT/8FMlN\r\n" \
//    "uQA5MgL2WGG57Riu8JFXRsG4pJ/9GY+21qOlAKBoEEd3/89dcZQ1BUzrN/mBoofp\r\n" \
//    "YJ/6RwwYR01zGUNq5FopOIBnToXo3FQvoeDhNNiNSUWo+k+lNRGbkR4u9ukiof9A\r\n" \
//    "oKVP37jqPbr1h+hhBDXKlsG5RlQeqk4hQOUb4hCkZzyw/q7AVORc+v+5NZD3dhOr\r\n" \
//    "AgMBAAGjYDBeMB8GA1UdIwQYMBaAFIo3m6hwzdX5SMiXfiGfWW9JjiQRMB0GA1Ud\r\n" \
//    "DgQWBBQ0Xji/lUxMDMNuvsXUGLDLDtgEjDAOBgNVHQ8BAf8EBAMCA/gwDAYDVR0T\r\n" \
//    "AQH/BAIwADANBgkqhkiG9w0BAQsFAAOCAQEAbVMHd3YxAwVCpuDB7ErsICMt3bvn\r\n" \
//    "hJ5kr57ShiSZ4gQsSrxC1bl0RTKpwFRzBTtau9Rqipt+XYtCmAFuTfA8hu8/P5jL\r\n" \
//    "X2yqV+Kwy0+EwOkHc8hOlYWnSZY8X/b0rxooG00HOMbpX8V24Om7Sa90Ky4AFxBr\r\n" \
//    "OVeYlQR9lWHiKbfrZzR0sUvS+QlLbzvjN8rDeA2Bn0UVDXT6bQrbEhV5N1qppDP6\r\n" \
//    "8DEtuwiH8RoHXBoXTE7cvb56nhn78EuoG4z0eX6Q7upS+TzAfx0wzvZqyz/mEz51\r\n" \
//    "t59jletxlyuFiBdylOUUCCTWJoaa1Q2BQIiQaAZ8kKNxg0JFGLSLDK4SNw==
//    "-----END CERTIFICATE-----"
"-----BEGIN CERTIFICATE-----\r\n" \
"MIIDhzCCAm+gAwIBAgIHUUkVSHFX5zANBgkqhkiG9w0BAQsFADBTMSgwJgYDVQQD\r\n" \
"DB9BbGliYWJhIENsb3VkIElvVCBPcGVyYXRpb24gQ0ExMRowGAYDVQQKDBFBbGli\r\n" \
"YWJhIENsb3VkIElvVDELMAkGA1UEBhMCQ04wIBcNMjAwOTE0MDIwODI1WhgPMjEy\r\n" \
"MDA5MTQwMjA4MjVaMFExJjAkBgNVBAMMHUFsaWJhYmEgQ2xvdWQgSW9UIENlcnRp\r\n" \
"ZmljYXRlMRowGAYDVQQKDBFBbGliYWJhIENsb3VkIElvVDELMAkGA1UEBhMCQ04w\r\n" \
"ggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCGFIa0xLlmiUl0e5t5DLbw\r\n" \
"uTTTEodcr+tCil0NYZI+1hVxD+75+rLRBrDoifURBIWU+MO59W1T8fKhto5f+AMo\r\n" \
"i4AZDDyL0H012W4PU6Ri0DPn0PsOAfgr7mYQv04UPxFR5BhKaxSl+ySva2Qd8KIb\r\n" \
"eFUDRIM5iv2GvJScwfsQAypZ2Z8LNMzSOLLuDoTcF3e+aH2XkztbM7D2fApztONx\r\n" \
"Non7YeVMF8/lp/jA8LxZ4gVb3xxEJsTAkwzURwasnDv3RzB3QQ9o9zRxVvZQOYu/\r\n" \
"gUiUbi6ET3nbPHMAlPWT+xvlXc9FeDSNmhjbl//dV9yI+bsbzoAO32PiLr9vHcXx\r\n" \
"AgMBAAGjYDBeMB8GA1UdIwQYMBaAFIo3m6hwzdX5SMiXfiGfWW9JjiQRMB0GA1Ud\r\n" \
"DgQWBBQz8TKNaFj52efYdIq1habhbYCq8zAOBgNVHQ8BAf8EBAMCA/gwDAYDVR0T\r\n" \
"AQH/BAIwADANBgkqhkiG9w0BAQsFAAOCAQEAXZiDjdC2vh5hogTVNY2+fmizhk8m\r\n" \
"hMy2p6u0FElC43aSzcY/7C6rlxLVWNF9eEzdh5qc9vJj1t3rCbREnGfa+7iwrnYG\r\n" \
"aRHnM8vmm5EYBU7A3J5gV0jX3eKSCiIXsNkKGX1pA/fL/NP0V8m+ihDkuOueHiAv\r\n" \
"iBfecVq7t5yohgOe9701pjiyCJZmjXWZTQa+4akfaBmD9jaxRGMtJFGsGCv1nNVt\r\n" \
"KIInpmn6z97fzVyEokEE7oHt7VC8YWMUj0vwGFUtfx0TiEW1o5trULkI55uLjmHV\r\n" \
"l5dcr5Zg2TiybKyvyFlCj9oRFMMSkI93y5bl4BNuDRcCLnLUDrNsIC8rJQ==\r\n" \
"-----END CERTIFICATE-----"

};

const char *g_cli_key = \
{
    \
//    "-----BEGIN RSA PRIVATE KEY-----\r\n"
//    "MIIEowIBAAKCAQEAkfFCt3sJV1tUdOcC3cy+1JBgDIgO7OhFRTal/fJ9bjx/MTFK\r\n" \
//    "uwBskobap9V98jQ4CdFH+PUbYZ7SFhPBZkVr5yPi0621pfJwp4Q46CrNhnsxih7v\r\n" \
//    "GbSRR3Ny2+dC6pL1CJCBkmwSjeGkq6E//BTJTbkAOTIC9lhhue0YrvCRV0bBuKSf\r\n" \
//    "/RmPttajpQCgaBBHd//PXXGUNQVM6zf5gaKH6WCf+kcMGEdNcxlDauRaKTiAZ06F\r\n" \
//    "6NxUL6Hg4TTYjUlFqPpPpTURm5EeLvbpIqH/QKClT9+46j269YfoYQQ1ypbBuUZU\r\n" \
//    "HqpOIUDlG+IQpGc8sP6uwFTkXPr/uTWQ93YTqwIDAQABAoIBAFCGn3pf4AKceRhb\r\n" \
//    "0uARNgZRdz3S4PFlI7uR8LsTTpRBuyioewR9smqTEBjlGq5Gk9kO6bh7fdU6bc8S\r\n" \
//    "rXbBvUz3JxQHWCAtim1T7pZj761RUj9U3DcBw9t53xlpHCoUVjGYknX5Pa+dnl+5\r\n" \
//    "BHYR/hTtjCsC5lGI6LtPEHyObpOJuEHhJnv944WmxCyLmshafIyOCU65xbNSBYPX\r\n" \
//    "Ua3riCDIBJDeKnJORzrFCoIOzoK3b6DLDeY1N/CdR/KqD+dWAAZmc8bfw3i8kk/1\r\n" \
//    "7cP2JiDUqeqRDG89rqTo5q7xIcsUDAQ4N30GawIAhAy3/CNxTX9/PYLkhGje4rr+\r\n" \
//    "mY0gbXECgYEAz9s1fjr0ihQ5oy0viFFLXYKJYQyRPOLYeQZP791Ss3nse/iUWWdc\r\n" \
//    "WvMxf2Pf6Flwo/dd4Rin/b1Ck/mrGBwM+NRnPgw6TNtP6zUIoVDh0hk6dcHZg8rM\r\n" \
//    "OBz5NAGf2+9FuuUUuoIrEBSwBnQdAi0zpIwL4Okj2RiMiYKk9NpCbrkCgYEAs77i\r\n" \
//    "JizJbKibh9Fdxkzjwn9onOEQHDbJZBNBuTmETzgZMhE2BQPBoLes4kxmnH1tA+AR\r\n" \
//    "DHKQUBcxyFs1i3MnnlVPl98G4tlqLLULyFVYF/q6s7grIHhTOA3uwR3jHEVlaGG+\r\n" \
//    "5p6vbMU+aj8M3xgQlGwFWieDyycvJy+jiyHlQ4MCgYEAtRqce0XG9+AzcBfYyH98\r\n" \
//    "mRc5t9OJYHGOh7SWRbqjajmbcVDfgR5r+9rVj9dnqjzzxgmsRIqFJLKYpfHtv24B\r\n" \
//    "Z3U6F3fz/C9CbP+ia1vbxoGwzBWA/jHeyi2EfGnF7Y2HRhZ/fxoXh0VfMeBNvi3A\r\n" \
//    "R2i2mNrKH0gie1XXgHgDVhECgYByYT233TgwFPsj6m8qCTPGSR1g9zcxPQvaywY/\r\n" \
//    "wAIaBzDBU4J0nndQtpElUEjbvFTq6mLu8Ng0nw3m2WOxYY4Mi20iz6GqKYAPAwaE\r\n" \
//    "bQqp97tnMgL/npIWUYdlDIfBLcfiLJE2YDRxmvKk2YpvTPj/+d1OCWcsl7rBzObd\r\n" \
//    "Wqxx0QKBgE5jHkgnaJz1GXBI1Md9LzuOZY9U8JiWJ93+ihMT/YSdVRzCMGRoCWRF\r\n" \
//    "4pDwZC+gcH/dgyC64H6SOkp7mVhtuHiFHP0AX/utGS3Z1h7dBYyuUJvUVITtHYkn\r\n" \
//    "eLmUqFxfbfIJG8V+viKJp0LsKPdpDVQRynI4KnnQ2SYlIYSDVI3r\r\n" \
//    "-----END RSA PRIVATE KEY-----"
"-----BEGIN RSA PRIVATE KEY-----\r\n"
"MIIEowIBAAKCAQEAhhSGtMS5ZolJdHubeQy28Lk00xKHXK/rQopdDWGSPtYVcQ/u\r\n" \
"+fqy0Qaw6In1EQSFlPjDufVtU/HyobaOX/gDKIuAGQw8i9B9NdluD1OkYtAz59D7\r\n" \
"DgH4K+5mEL9OFD8RUeQYSmsUpfskr2tkHfCiG3hVA0SDOYr9hryUnMH7EAMqWdmf\r\n" \
"CzTM0jiy7g6E3Bd3vmh9l5M7WzOw9nwKc7TjcTaJ+2HlTBfP5af4wPC8WeIFW98c\r\n" \
"RCbEwJMM1EcGrJw790cwd0EPaPc0cVb2UDmLv4FIlG4uhE952zxzAJT1k/sb5V3P\r\n" \
"RXg0jZoY25f/3VfciPm7G86ADt9j4i6/bx3F8QIDAQABAoIBAFAoPVAcu+o88Rke\r\n" \
"PJhlrYjEcHwve5VB5pflODQjvNasDi2FpQz1f6nPIjwjHcuKtIGXJwwpWa7x2mgr\r\n" \
"R3rAPsMigvZ2hkvf3LswvceT7ZbVefKA0VZSfxdkcshYVklsWuJmRs9WrQV92zS0\r\n" \
"MypjDcb2bXyCnSDtoxvF6eK9ZWbwW7yPIhXpkUwin8sVdhWpZTtzOI6gbEa+EBYl\r\n" \
"+UDcNJDcbSbX8wCPN/bRiwrIfH1YrXyACpOSfOZ++8EZdtERy4n9xCVpNrqLf7zj\r\n" \
"k7wGO5RkHLRyiS9vVcL2ck63CTO9GjZZSGjoxGUIHn9L2PAzoaLyCFLY6BjE2wWa\r\n" \
"qlOiFUECgYEA4VoNWRLQ2bWc6NwScmFfGnrhMraK/tZ17U4jx8xCAvhhx4AG+lms\r\n" \
"gH0HMBYHHhOiWR0a1fC4xFiAS8XfBryTwLUFYHCZcpQZrRZQK39348JBiyQkDnZM\r\n" \
"/w9fld/oXVZv+vlMtEHjpTQnFb5ukOhkO3ZwEEw2tlLcznEMIddJSO0CgYEAmFC4\r\n" \
"wj01TW0HZA0WzSxKQvCErcp4eHq9+yR57VrBvs1ZBQ3xjWdWDvHOEKS4smMRoRPX\r\n" \
"fZRYEhFhshR2N9oF1nlecYY9oZBL8AAu2rVn1IHqjmqs4bU/yVHBSEAots0XnzAM\r\n" \
"nkeapB2YT53+9hQaEBtE71VLqVCqCX6wTFJpJJUCgYB5aVgZrm6BbTytpKv3nghs\r\n" \
"LtoXxYubgxYkFV5nrd++0+1yW8LmrNuomjP+L1AEX1Wvp2Om8jvJ5Vix+liPu5fr\r\n" \
"UlwkOmYh+jLfM09cMB/6wNUpTv5yIECZhsHb8LezWfeZAynVUE6C2ldYerH9NoeS\r\n" \
"FdyIm1c64ef+/Z8ouGHOfQKBgHrue4/ZktGgs5xerA6ANpd8Q7l4dm7Wscqmj/mv\r\n" \
"jDuQxM49iL1Vr7F0G5p3N9JR/wKAXyWtur8qZC00Mo21W4zUfBMoA0oIiiF0JFzE\r\n" \
"mL589HBpTIsGnCrB3zuO/M09DlmekDCXHgpjEHEB1auKYdaqC6MWtnA6+GGdbHDM\r\n" \
"+qtZAoGBAKXtiTqtL5lz0uv6RiUW4IddpUK/3KV3RzHpbbdlxNVgxqknu/ytbPU1\r\n" \
"nzivqn9qBRd+ufz8O8OEJel7lkF3gNkQA6heNLYeSdho1DbnjhwdD6fZufT9C7iS\r\n" \
"qjaA1+FdxFCWVkIZgHSLMSK0IKJJeCoO8HKA3RfpU0DLRSnMBkmm\r\n" \
"-----END RSA PRIVATE KEY-----"
};

uintptr_t HAL_SSL_Establish(const char *host,
                            uint16_t port,
                            const char *ca_crt,
                            uint32_t ca_crt_len)
{
    char                port_str[6];
    const char         *alter = host;
    TLSDataParams_pt    pTlsData;

    if (host == NULL || ca_crt == NULL) {
        LOG_E(DBG_EXT_TAG, "input params are NULL, abort");
        return 0;
    }

    if (!strlen(host) || (strlen(host) < 8)) {
        LOG_E(DBG_EXT_TAG, "invalid host: '%s'(len=%d), abort", host, (int)strlen(host));
        return 0;
    }

    pTlsData = g_ssl_hooks.malloc(sizeof(TLSDataParams_t));
    if (NULL == pTlsData) {
        LOG_E(DBG_EXT_TAG, "pTlsData malloc failed(size:%d),abort",sizeof(TLSDataParams_t));
        return (uintptr_t)NULL;
    }
    memset(pTlsData, 0x0, sizeof(TLSDataParams_t));

    sprintf(port_str, "%u", port);

    //mbedtls_platform_set_calloc_free(_SSLCalloc_wrapper, _SSLFree_wrapper);

    if (0 != _TLSConnectNetwork(pTlsData, alter, port_str, ca_crt, ca_crt_len, g_cli_crt, strlen(g_cli_crt) + 1, g_cli_key, strlen(g_cli_key) + 1, NULL, 0)) {
        _network_ssl_disconnect(pTlsData);
        g_ssl_hooks.free((void *)pTlsData);
        return (uintptr_t)NULL;
    }

    return (uintptr_t)pTlsData;
}

int HAL_SSL_Read(uintptr_t handle, char *buf, int len, int timeout_ms)
{
    return _network_ssl_read((TLSDataParams_t *)handle, buf, len, timeout_ms);;
}

int HAL_SSL_Write(uintptr_t handle, const char *buf, int len, int timeout_ms)
{
    return _network_ssl_write((TLSDataParams_t *)handle, buf, len, timeout_ms);
}

int32_t HAL_SSL_Destroy(uintptr_t handle)
{
    if ((uintptr_t)NULL == handle) {
        LOG_I(DBG_EXT_TAG, "handle is NULL");
        return 0;
    }

    _network_ssl_disconnect((TLSDataParams_t *)handle);
    g_ssl_hooks.free((void *)handle);
    return 0;
}
