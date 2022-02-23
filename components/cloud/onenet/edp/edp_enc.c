#include "edp_enc.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <bignum.h>
#include <rsa.h>
#include <aes.h>
#include <cmac.h>
#include <os_types.h>
#include <os_mq.h>
#include <os_errno.h>
#include <os_task.h>
#include <os_assert.h>

#define RSA_E_LEN_BYTE                          4
#define RSA_N_LEN_BYTE                          128
#define RSA_E                                   65537
#define RSA_PADDING_LEN                         11

#define AES_KEY_LEN                             128

//static RSA* g_rsa = NULL;
mbedtls_rsa_context g_rsa;

mbedtls_aes_context aes_dec_ctx;
mbedtls_aes_context aes_enc_ctx;

static EncryptAlgType g_encrypt_alg_type = kTypeAes;

static int myrand( void *rng_state, unsigned char *output, size_t len )
{
    size_t i;

    if( rng_state != NULL )
        rng_state  = NULL;

    for( i = 0; i < len; ++i )
        output[i] = rand();

    return( 0 );
}

EdpPacket* PacketEncryptReq(EncryptAlgType type){
    int ret = 0;
    EdpPacket* pkg = NULL;
    unsigned remainlen = 0;
    unsigned char tmp[RSA_E_LEN_BYTE] = {0};
    //memset(tmp,0,RSA_E_LEN_BYTE+1);

    mbedtls_mpi K;

    if (type != kTypeAes){
    	return NULL;
    }

    /* init rsa */
    mbedtls_mpi_init( &K );
    mbedtls_rsa_init( &g_rsa, MBEDTLS_RSA_PKCS_V15, 0 );

	ret=mbedtls_rsa_gen_key(&g_rsa, myrand,NULL, RSA_N_LEN_BYTE * 8, RSA_E);
	if (0 != ret){
	    return NULL;
	}
    
    if( mbedtls_rsa_check_pubkey(  &g_rsa ) != 0 ||
        mbedtls_rsa_check_privkey( &g_rsa ) != 0 )
    {
        os_kprintf("check rsa key error\r\n");
	    return NULL;        
    }

    pkg = NewBuffer();
    remainlen = RSA_N_LEN_BYTE + RSA_E_LEN_BYTE + 1; 
    WriteByte(pkg, ENCRYPTREQ);
    WriteRemainlen(pkg, remainlen);
    
    //pkg->_data[pkg->_write_pos++]=(char)(remainlen&0xff);
    //pkg->_data[pkg->_write_pos++]=(char)((remainlen>>8)&0xff);

    memset(pkg->_data + pkg->_write_pos, 0, RSA_N_LEN_BYTE + RSA_E_LEN_BYTE + 1);
    
    /* write e */
    ret = mbedtls_mpi_write_binary(&g_rsa.E, &tmp[0], RSA_E_LEN_BYTE);
    if (0 != ret){
    	DeleteBuffer(&pkg);
    	return NULL;
    }
    tmp[0]=0;
    
    memcpy(pkg->_data + pkg->_write_pos,tmp, RSA_E_LEN_BYTE);
    pkg->_write_pos += RSA_E_LEN_BYTE;

    /* write n */
    ret = mbedtls_mpi_write_binary(&g_rsa.N, pkg->_data + pkg->_write_pos, RSA_N_LEN_BYTE);
    if (0 != ret){
    	DeleteBuffer(&pkg);
    	return NULL;
    }
    
    pkg->_write_pos += RSA_N_LEN_BYTE;
    
    /* 
     * The type of symmetric encryption algorithm.
     * Right now, only support AES whose code is 1
     */
    g_encrypt_alg_type = type;
    WriteByte(pkg, g_encrypt_alg_type); 		

    return pkg;
}

int32 UnpackEncryptResp(EdpPacket* pkg){
    uint32 remainlen = 0;
    uint16 key_len = 0;

    int ret = 0;
    unsigned char key[BUFFER_SIZE] = {0};
    unsigned int len = 0;

    if (ReadRemainlen(pkg, &remainlen))
	return ERR_UNPACK_ENCRYPT_RESP;

    if (ReadUint16(pkg, &key_len))
	return ERR_UNPACK_ENCRYPT_RESP;
    
    if (remainlen != key_len + 2)
	return ERR_UNPACK_ENCRYPT_RESP;

    ret = mbedtls_rsa_pkcs1_decrypt( &g_rsa, myrand, NULL, MBEDTLS_RSA_PRIVATE,
                                       &len, pkg->_data + pkg->_read_pos, key,
                                       BUFFER_SIZE);
    if (0 != ret){
        return ERR_UNPACK_ENCRYPT_RESP;
    }

    os_kprintf("AES KEY:%s\r\n", key);
    mbedtls_aes_init( &aes_enc_ctx );
    mbedtls_aes_init( &aes_dec_ctx );

    switch (g_encrypt_alg_type){
    case kTypeAes:
        if(0 != mbedtls_aes_setkey_dec( &aes_dec_ctx, key, AES_KEY_LEN )) {
            return ERR_UNPACK_ENCRYPT_RESP;
        }

        if(0 != mbedtls_aes_setkey_enc( &aes_enc_ctx, key, AES_KEY_LEN )) {
            return ERR_UNPACK_ENCRYPT_RESP;
        }
        break;

    default:
        return ERR_UNPACK_ENCRYPT_RESP;
    }

    return 0;
}

/*************** AES ***************/
static int aes_encrypt(EdpPacket* pkg, int remain_pos){
    uint32 in_len = 0;
    unsigned char* in = NULL;
    unsigned char* out = NULL;
    size_t div = 0;
    size_t mod = 0;
    size_t adder = 0;
    size_t block = 0;
    size_t padding_len = 0;
    unsigned char* padding_addr = NULL;
    size_t len_aft_enc =  0;
    uint8 tmp_buf[5] = {0};
    EdpPacket tmp_pkg;
    int diff = 0;
    int i = 0;

    /* 加密 */
    in_len = pkg->_write_pos - remain_pos;
    in = pkg->_data + remain_pos;
    /* AES 支持加密后数据存放在加密前的buffer中 */
    out = in;
    
    div = in_len / MBEDTLS_AES_BLOCK_SIZE;
    mod = in_len % MBEDTLS_AES_BLOCK_SIZE;

    padding_len = MBEDTLS_AES_BLOCK_SIZE - mod;
    padding_addr = in + div * MBEDTLS_AES_BLOCK_SIZE;
    if (mod){
        padding_addr += mod;
    }
    ++div;
    /* 填充
     * (1) 如果被加密数据长度刚好是AES_BLOCK_SIZE的整数倍，则在后面
     *  填充AES_BLOCK_SIZE个'0' + AES_BLOCK_SIZE，如果AES_BLOCK_SIZE等于
     *  16，则填充16个 '0'+16
     * (2) 如果不是整数倍，假设最后一段长度为n，则在末尾填充AES_BLOCK_SIZE-n
     * 个'0' + AES_BLOCK_SIZE - n。比如AES_BLOCK_SIZE = 16, n=11, 则在最后
     * 填充5个 '0'+5.
     */
    memset(padding_addr, '0' + padding_len, padding_len);

    /* AES 支持加解密前后存放在同一buffer中 */
    for (block=0; block<=div; ++block){
        adder = block * MBEDTLS_AES_BLOCK_SIZE;
        //AES_encrypt(in + adder, out + adder, &g_aes_encrypt_key);
        if(0 !=mbedtls_aes_crypt_ecb( &aes_enc_ctx, MBEDTLS_AES_ENCRYPT, in + adder, out + adder))
            return -1;
    }
    
    /* 加密后的remainlen会变得大，其占用的空间可能会变大
     * 利用一个临时的EdpPacket来测试加密后remainlen的长度是否发生改变
     * 若改变，则加密后的数据应该依次往后移，以为remainlen留出足够空间
     */
    len_aft_enc =  div * MBEDTLS_AES_BLOCK_SIZE;

    tmp_pkg._data = tmp_buf;
    tmp_pkg._write_pos = 1;
    tmp_pkg._read_pos = 0;
    WriteRemainlen(&tmp_pkg, len_aft_enc);
    diff = tmp_pkg._write_pos - remain_pos;
    if (diff > 0){
        i = len_aft_enc;
        for (; i>0; i--){
            *(in + i + diff - 1)  = *(in + i - 1);
        }
    }
    
    pkg->_write_pos = 1;
    pkg->_read_pos = 0;
    WriteRemainlen(pkg, len_aft_enc);
    pkg->_write_pos += len_aft_enc;

    return len_aft_enc;
}

static int aes_decrypt(EdpPacket* pkg, int remain_pos){
    size_t in_len = 0;
    unsigned char* in = NULL;
    unsigned char* out = NULL;
    size_t offset = 0;
    size_t padding_len = 0;
    uint32 len_aft_dec = 0;
    uint8 tmp_buf[5] = {0};
    EdpPacket tmp_pkg;
    int diff = 0;
    int i = 0;

    in_len = pkg->_write_pos - pkg->_read_pos;
    in = pkg->_data + pkg->_read_pos;
    out = in;

    for (offset=0; (offset+MBEDTLS_AES_BLOCK_SIZE)<=in_len; offset+=MBEDTLS_AES_BLOCK_SIZE){
        //AES_decrypt(in + offset, out + offset, &g_aes_decrypt_key);
        if(0 !=mbedtls_aes_crypt_ecb( &aes_dec_ctx, MBEDTLS_AES_DECRYPT, in + offset, out + offset))
            return -1;
    }

    padding_len = *(in + offset -1) - '0';
    if (padding_len > MBEDTLS_AES_BLOCK_SIZE){
        return -1;
    }

    /* 解密后的remainlen会变小，其占用空间可能变小
     * 利用一个临时的EdpPacket来测试加密后remainlen的长度是否发生改变
     * 若改变，则解密后的数据应该依次往前移，以消除多余空间
     */
    len_aft_dec = offset - padding_len;
    tmp_pkg._data = tmp_buf;
    tmp_pkg._write_pos = 1;
    tmp_pkg._read_pos = 0;
    WriteRemainlen(&tmp_pkg, len_aft_dec);

    diff = remain_pos - tmp_pkg._write_pos;
    if (diff > 0){
        i = 0;
        for (i=0; i<len_aft_dec; i++){
            *(in + i - diff)  = *(in + i);
        }
    }
    
    pkg->_write_pos = 1;
    pkg->_read_pos = 0;
    WriteRemainlen(pkg, len_aft_dec);
    pkg->_read_pos = 1;
    pkg->_write_pos += len_aft_dec;

    return len_aft_dec;
}

/*************** end AES ***************/

int SymmEncrypt(EdpPacket* pkg){
    int ret = 0;
    uint32 remain_len = 0;
    uint32 remain_pos = 0;

    pkg->_read_pos = 1;
    ReadRemainlen(pkg, &remain_len);
    assert(remain_len == (pkg->_write_pos - pkg->_read_pos));
    if (remain_len == 0){	/* no data for encrypting */
	pkg->_read_pos = 1;
        return 0;
    }
    remain_pos = pkg->_read_pos;

    switch (g_encrypt_alg_type){
    case kTypeAes:
        ret = aes_encrypt(pkg, remain_pos);
        break;

    default:
        ret = -1;
        break;
    }

    return ret;
}

int SymmDecrypt(EdpPacket* pkg){
    int ret = 0;
    uint32 remain_len = 0;
    uint32 remain_pos = 0;

    pkg->_read_pos = 1;
    ReadRemainlen(pkg, &remain_len);
    assert(remain_len == (pkg->_write_pos - pkg->_read_pos));
    if (remain_len == 0){	/* no data for decrypting */
        pkg->_read_pos = 1;
        return 0;
    }
    remain_pos = pkg->_read_pos;

    switch (g_encrypt_alg_type){
    case kTypeAes:
        ret = aes_decrypt(pkg, remain_pos);
        break;

    default:
        ret = -1;
        break;
    }

    return ret;
}
