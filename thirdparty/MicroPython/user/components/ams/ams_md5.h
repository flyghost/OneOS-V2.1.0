#ifndef _AMS_MD5_H
#define _AMS_MD5_H

#include "stdint.h"
#include "model_def.h"



/* Data structure for MD5 (Message-Digest) computation */
typedef struct {
  u32_t i[2];               /* number of _bits_ handled mod 2^64 */
  u32_t buf[4];             /* scratch buffer */
  unsigned char in[64];     /* input buffer */
  unsigned char digest[16]; /* actual digest after MD5Final call */
} MD5_CTX;

void MD5Init  ( MD5_CTX *mdContext);
void MD5Update( MD5_CTX *mdContext, unsigned char *inBuf, unsigned int inLen);
void MD5Final ( unsigned char hash[], MD5_CTX *mdContext);

#endif /* MD5_H */

