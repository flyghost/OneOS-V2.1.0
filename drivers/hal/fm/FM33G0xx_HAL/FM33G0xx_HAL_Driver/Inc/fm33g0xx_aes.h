/**
  ******************************************************************************
  * @file    fm33g0xx_aes.h
  * @author  FM33g0xx Application Team
  * @version V0.3.00G
  * @date    08-31-2018
  * @brief   This file contains all the functions prototypes for the AES firmware library.  
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FM33G0XX_AES_H
#define __FM33G0XX_AES_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "FM33G0XX.h"
	 
/** @addtogroup FM33G0xx_StdPeriph_Driver
  * @{
  */
typedef struct
{
	uint32_t	KEYLEN;			//AES������Կ���ȣ�AESEN=1ʱ�����޸�
	uint32_t	CHMOD;			//AES����������ģʽ��AESEN=1ʱ�����޸�
	uint32_t	MODE;			//AES����ģʽ��AESEN=1ʱ�����޸�
	uint32_t	DATATYP;		//ѡ���������ͣ�AESEN=1ʱ�����޸ġ����彻������ɲο�AES���������½�	
	FunState	DMAOEN;			//DMA�����Զ�����ʹ��
	FunState	DMAIEN;			//DMA�����Զ�д��ʹ��
	FunState	ERRIE;			//�����־��RDERR��WRERR���ж�ʹ��
	FunState	CCFIE;			//CCF��־�ж�ʹ��
	FunState	AESEN;			//AESʹ�� 
	
}AES_InitTypeDef;
	 
	 



#define	AES_AESCR_KEYLEN_Pos	13	/* AES������Կ���ȣ�AESEN=1ʱ�����޸� */
#define	AES_AESCR_KEYLEN_Msk	(0x3U << AES_AESCR_KEYLEN_Pos)
#define	AES_AESCR_KEYLEN_128BIT	(0x0U << AES_AESCR_KEYLEN_Pos)	/* 00��128bit */
#define	AES_AESCR_KEYLEN_192BIT	(0x1U << AES_AESCR_KEYLEN_Pos)	/* 01��192bit */
#define	AES_AESCR_KEYLEN_256BIT	(0x2U << AES_AESCR_KEYLEN_Pos)	/* 10��256bit */

#define	AES_AESCR_DMAOEN_Pos	12	/* DMA�����Զ�����ʹ�� */
#define	AES_AESCR_DMAOEN_Msk	(0x1U << AES_AESCR_DMAOEN_Pos)
	/* 0�������� */
	/* 1������ */

#define	AES_AESCR_DMAIEN_Pos	11	/* DMA�����Զ�д��ʹ�� */
#define	AES_AESCR_DMAIEN_Msk	(0x1U << AES_AESCR_DMAIEN_Pos)
	/* 0�������� */
	/* 1������ */

#define	AES_AESCR_ERRIE_Pos	10	/* �����־��RDERR��WRERR���ж�ʹ�� */
#define	AES_AESCR_ERRIE_Msk	(0x1U << AES_AESCR_ERRIE_Pos)
	/* 0����ʹ�� */
	/* 1��ʹ�� */

#define	AES_AESCR_CCFIE_Pos	9	/* CCF��־�ж�ʹ�� */
#define	AES_AESCR_CCFIE_Msk	(0x1U << AES_AESCR_CCFIE_Pos)
	/* 0����ʹ�� */
	/* 1��ʹ�� */

#define	AES_AESCR_ERRC_Pos	8	/* ��������־д1���WRERR��RDERR�����־ */
#define	AES_AESCR_ERRC_Msk	(0x1U << AES_AESCR_ERRC_Pos)

#define	AES_AESCR_CCFC_Pos	7	/* ���CCF��־д1��CCF��־
 */
#define	AES_AESCR_CCFC_Msk	(0x1U << AES_AESCR_CCFC_Pos)

#define	AES_AESCR_CHMOD_Pos	5	/* AES����������ģʽ��AESEN=1ʱ�����޸� */
#define	AES_AESCR_CHMOD_Msk	(0x3U << AES_AESCR_CHMOD_Pos)
#define	AES_AESCR_CHMOD_ECB	(0x0U << AES_AESCR_CHMOD_Pos)	/* 00��ECB */
#define	AES_AESCR_CHMOD_CBC	(0x1U << AES_AESCR_CHMOD_Pos)	/* 01��CBC */
#define	AES_AESCR_CHMOD_CTR	(0x2U << AES_AESCR_CHMOD_Pos)	/* 10��CTR */
#define	AES_AESCR_CHMOD_MULTH	(0x3U << AES_AESCR_CHMOD_Pos)	/* 11��ʹ��MultHģ�� */

#define	AES_AESCR_MODE_Pos	3	/* AES����ģʽ��AESEN=1ʱ�����޸� */
#define	AES_AESCR_MODE_Msk	(0x3U << AES_AESCR_MODE_Pos)
#define	AES_AESCR_MODE_ENCRYPT	(0x0U << AES_AESCR_MODE_Pos)	/* 00��ģʽ1������ */
#define	AES_AESCR_MODE_KEYEXP	(0x1U << AES_AESCR_MODE_Pos)	/* 01��ģʽ2����Կ��չ */
#define	AES_AESCR_MODE_DECRYPT	(0x2U << AES_AESCR_MODE_Pos)	/* 10��ģʽ3������ */
#define	AES_AESCR_MODE_KEYEXPDECRYPT	(0x3U << AES_AESCR_MODE_Pos)	/* 11��ģʽ4����Կ��չ+���� */

#define	AES_AESCR_DATATYP_Pos	1	/* ѡ���������ͣ�AESEN=1ʱ�����޸� */
#define	AES_AESCR_DATATYP_Msk	(0x3U << AES_AESCR_DATATYP_Pos)
#define	AES_AESCR_DATATYP_32BITNOEX	(0x0U << AES_AESCR_DATATYP_Pos)	/* 00��32bit���ݲ����� */
#define	AES_AESCR_DATATYP_16BITEX	(0x1U << AES_AESCR_DATATYP_Pos)	/* 01��16bit���ݰ��ֽ��� */
#define	AES_AESCR_DATATYP_8BITEX	(0x2U << AES_AESCR_DATATYP_Pos)	/* 10��8bit�����ֽڽ��� */
#define	AES_AESCR_DATATYP_1BITEX	(0x3U << AES_AESCR_DATATYP_Pos)	/* 11��1bit���ݱ��ؽ��� */

#define	AES_AESCR_AESEN_Pos	0	/* AESʹ��,���κ�ʱ�����AESENλ���ܹ���λAESģ�� */
#define	AES_AESCR_AESEN_Msk	(0x1U << AES_AESCR_AESEN_Pos)
	/* 0����ʹ�� */
	/* 1��ʹ�� */

#define	AES_AESIF_WRERR_Pos	2	/* д�����־���ڼ��������׶η���д����ʱ��λ�������ERRC�Ĵ���д1���� */
#define	AES_AESIF_WRERR_Msk	(0x1U << AES_AESIF_WRERR_Pos)

#define	AES_AESIF_RDERR_Pos	1	/* �������־���ڼ��������׶η���������ʱ��λ�������ERRC�Ĵ���д1���� */
#define	AES_AESIF_RDERR_Msk	(0x1U << AES_AESIF_RDERR_Pos)

#define	AES_AESIF_CCF_Pos	0	/* AES������ɱ�־������ͨ��CCFC��1�������жϱ�־ */
#define	AES_AESIF_CCF_Msk	(0x1U << AES_AESIF_CCF_Pos)

#define	AES_AESDIN_AESDIN_Pos	0	/* ��������Ĵ�������AES��Ҫ����ӽ�������ʱ��Ӧ�����üĴ�������д4�� */
#define	AES_AESDIN_AESDIN_Msk	(0xffffffffU << AES_AESDIN_AESDIN_Pos)

#define	AES_AESDOUT_AESDOUT_Pos	0	/* ��������Ĵ�������AES������ɺ󣬿��Է��Ĵζ����ӽ��ܵĽ�� */
#define	AES_AESDOUT_AESDOUT_Msk	(0xffffffffU << AES_AESDOUT_AESDOUT_Pos)


//Macro_End

/* Exported functions --------------------------------------------------------*/ 
extern void AES_Deinit(void);

/* AES������Կ���ȣ�AESEN=1ʱ�����޸� ��غ��� */
extern void AES_AESCR_KEYLEN_Set(uint32_t SetValue);
extern uint32_t AES_AESCR_KEYLEN_Get(void);

/* DMA�����Զ�����ʹ�� ��غ��� */
extern void AES_AESCR_DMAOEN_Setable(FunState NewState);
extern FunState AES_AESCR_DMAOEN_Getable(void);

/* DMA�����Զ�д��ʹ�� ��غ��� */
extern void AES_AESCR_DMAIEN_Setable(FunState NewState);
extern FunState AES_AESCR_DMAIEN_Getable(void);

/* �����־��RDERR��WRERR���ж�ʹ�� ��غ��� */
extern void AES_AESCR_ERRIE_Setable(FunState NewState);
extern FunState AES_AESCR_ERRIE_Getable(void);

/* CCF��־�ж�ʹ�� ��غ��� */
extern void AES_AESCR_CCFIE_Setable(FunState NewState);
extern FunState AES_AESCR_CCFIE_Getable(void);

/* ��������־д1���WRERR��RDERR�����־ ��غ��� */
extern void AES_AESCR_ERRC_Clr(void);

/* ���CCF��־д1��CCF��־
 ��غ��� */
extern void AES_AESCR_CCFC_Clr(void);

/* AES����������ģʽ��AESEN=1ʱ�����޸� ��غ��� */
extern void AES_AESCR_CHMOD_Set(uint32_t SetValue);
extern uint32_t AES_AESCR_CHMOD_Get(void);

/* AES����ģʽ��AESEN=1ʱ�����޸� ��غ��� */
extern void AES_AESCR_MODE_Set(uint32_t SetValue);
extern uint32_t AES_AESCR_MODE_Get(void);

/* ѡ���������ͣ�AESEN=1ʱ�����޸� ��غ��� */
extern void AES_AESCR_DATATYP_Set(uint32_t SetValue);
extern uint32_t AES_AESCR_DATATYP_Get(void);

/* AESʹ��,���κ�ʱ�����AESENλ���ܹ���λAESģ�� ��غ��� */
extern void AES_AESCR_AESEN_Setable(FunState NewState);
extern FunState AES_AESCR_AESEN_Getable(void);

/* д�����־���ڼ��������׶η���д����ʱ��λ�������ERRC�Ĵ���д1���� ��غ��� */
extern FlagStatus AES_AESIF_WRERR_Chk(void);

/* �������־���ڼ��������׶η���������ʱ��λ�������ERRC�Ĵ���д1���� ��غ��� */
extern FlagStatus AES_AESIF_RDERR_Chk(void);

/* AES������ɱ�־������ͨ��CCFC��1�������жϱ�־ ��غ��� */
extern FlagStatus AES_AESIF_CCF_Chk(void);

/* ��������Ĵ�������AES��Ҫ����ӽ�������ʱ��Ӧ�����üĴ�������д4�� ��غ��� */
extern void AES_AESDIN_Write(uint32_t SetValue);
extern uint32_t AES_AESDIN_Read(void);

/* ��������Ĵ�������AES������ɺ󣬿��Է��Ĵζ����ӽ��ܵĽ�� ��غ��� */
extern void AES_AESDOUT_Write(uint32_t SetValue);
extern uint32_t AES_AESDOUT_Read(void);
//Announce_End

/*�ӽ�����Կ���룬key0�����Կ���32bit */
extern void AES_AESKEY_WriteEx(uint8_t *KeyIn, uint8_t Len);

/*�ӽ�����Կ�������һ���ִ����Կ���32bit  */
extern void AES_AESKEY_ReadEx(uint8_t *KeyOut, uint8_t Len);

/*��ʼ�������� */
extern void AES_AESIVR_WriteEx(uint8_t *IVRIn);

/*��ʼ������ȡ */
extern void AES_AESIVR_ReadEx(uint8_t *IVROut);

/*�ӽ����������룬16�ֽڣ�128bit��������������  */
extern void AES_AESDIN_GroupWrite_128BIT(uint8_t *DataIn);
   						
/*�ӽ������������16�ֽڣ�128bit�������������  */
extern void AES_AESDOUT_GroupRead_128BIT(uint8_t *DataOut);

extern uint8_t AES_GroupWriteAndRead_128BIT(uint8_t *DataIn, uint8_t *DataOut);

/* AESģ���ʼ������ */
extern void AES_Init(AES_InitTypeDef* para);

#ifdef __cplusplus
}
#endif

#endif



/************************ (C) COPYRIGHT FMSHelectronics *****END OF FILE****/



