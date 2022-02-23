#ifndef HTTP_API_H
#define HTTP_API_H


#ifndef HTTP_CLIENT_MAX_SCHEME_LEN
#define HTTP_CLIENT_MAX_SCHEME_LEN  (8)
#endif

#ifndef HTTP_CLIENT_MAX_HOST_LEN
#define HTTP_CLIENT_MAX_HOST_LEN   (64)
#endif

#ifndef HTTP_CLIENT_MAX_URL_LEN
#define HTTP_CLIENT_MAX_URL_LEN    (512)
#endif

#ifndef HTTP_CLIENT_MAX_CONTENT_RANGE_LEN
#define HTTP_CLIENT_MAX_CONTENT_RANGE_LEN (20)
#endif

#ifndef HTTP_PORT
#define HTTP_PORT   (80)
#endif

#ifndef HTTPS_PORT
#define HTTPS_PORT (443)
#endif

#ifndef HTTP_CLIENT_SEND_BUF_SIZE
#define HTTP_CLIENT_SEND_BUF_SIZE  (1024)          /* send */
#endif

#ifndef HTTP_CLIENT_CHUNK_SIZE
#ifdef HTTP_REQUEST_BLOCK_SIZE
#define HTTP_CLIENT_CHUNK_SIZE     (64 + HTTP_REQUEST_BLOCK_SIZE)
#else
#define HTTP_CLIENT_CHUNK_SIZE     1024
#endif
#endif

#ifndef HTTP_CLIENT_AUTHB_SIZE
#define HTTP_CLIENT_AUTHB_SIZE     128
#endif


/** @brief   http error code */
typedef enum {
    HTTP_EAGAIN   =  1,  /**< more data to retrieved */
    HTTP_SUCCESS  =  0,  /**< operation success      */
    HTTP_ENOBUFS  = -1,  /**< buffer error           */
    HTTP_EARG     = -2,  /**< illegal argument       */
    HTTP_ENOTSUPP = -3,  /**< not support            */
    HTTP_EDNS     = -4,  /**< DNS fail               */
    HTTP_ECONN    = -5,  /**< connect fail           */
    HTTP_ESEND    = -6,  /**< send packet fail       */
    HTTP_ECLSD    = -7,  /**< connect closed         */
    HTTP_ERECV    = -8,  /**< recv packet fail       */
    HTTP_EPARSE   = -9,  /**< url parse error        */
    HTTP_EPROTO   = -10, /**< protocol error         */
    HTTP_EUNKOWN  = -11, /**< unknown error          */
    HTTP_ETIMEOUT = -12, /**< timeout                */
} HTTP_RESULT_CODE;





#endif
