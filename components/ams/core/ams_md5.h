#ifndef __AMS_MD5_H__
#define __AMS_MD5_H__
#include <stdint.h>
/* Data structure for MD5 (Message-Digest) computation */
typedef struct {
  uint32_t i[2];                /* number of _bits_ handled mod 2^64 */
  uint32_t buf[4];              /* scratch buffer */
  uint8_t digest[16];           /* actual digest after MD5Final call */
  uint8_t in[64];               /* input buffer */
} MD5_CTX;

void MD5Init  ( MD5_CTX *mdContext);
void MD5Update( MD5_CTX *mdContext, unsigned char *inBuf, unsigned int inLen);
void MD5Final ( unsigned char hash[], MD5_CTX *mdContext);


#endif /* __AMS_MD5_H__ */

