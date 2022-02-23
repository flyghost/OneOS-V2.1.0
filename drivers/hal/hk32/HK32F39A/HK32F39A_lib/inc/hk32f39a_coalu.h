/**
  ******************************************************************************
  * @file    hk32f39a_coalu.h
  * @author  laura.c    
  * @version V1.0.0
  * @date    2019-10-18
  * @brief   This file contains all the functions prototypes for the CO_ALU firmware 
  *          library.
  ****************************************************************************** 
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HK32F39A_COALU_H
#define __HK32F39A_COALU_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "hk32f39a.h"

//#define USE_HK_FLOAT_POINT_FUNC	


#if  defined ( __CC_ARM )
  #define __SIMD32_TYPE int32_t __packed
  #define CMSIS_UNUSED __attribute__((unused))
  #define CMSIS_INLINE __attribute__((always_inline))
	
#elif defined ( __ICCARM__ )
  #define CMSIS_INLINE        inline 
#endif


/**
 * @brief 8-bit fractional data type in 1.7 format.
 */
typedef int8_t q7_t;

/**
 * @brief 16-bit fractional data type in 1.15 format.
 */
typedef int16_t q15_t;

/**
 * @brief 32-bit fractional data type in 1.31 format.
 */
typedef int32_t q31_t;

/**
 * @brief 64-bit fractional data type in 1.63 format.
 */
typedef int64_t q63_t;

/**
 * @brief 32-bit floating-point type definition.
 */
typedef float float32_t;

/*COALU INTER define*/
#define COALU_INTER_Mask		((uint32_t)0x1F000000)	
#define COALU_INTER_DIVOFEN		((uint32_t)0x10000000)		//除法溢出中断使能
#define COALU_INTER_DZIEN		((uint32_t)0x08000000)		//除数为0中断使能
#define COALU_INTER_IOIEN		((uint32_t)0x04000000)		//非法操作中断使能
#define COALU_INTER_QFIEN		((uint32_t)0x02000000)		//饱和异常中断使能
#define COALU_INTER_OFIEN		((uint32_t)0x01000000)		//溢出异常中断使能
#define COALU_INTER_NONE		((uint32_t)0x00000000)		//失能协处理器中断

#define COALU_START				((uint32_t)0x00020000)	

/*COALU RMODE define*/
#define COALU_RMODE_Mask		((uint32_t)0x00300000)	
#define COALU_RMODE_RN			((uint32_t)0x00000000)	//舍入到最近
#define COALU_RMODE_RP			((uint32_t)0x00100000)	//舍入到正无穷
#define COALU_RMODE_RM			((uint32_t)0x00200000)	//舍入到负无穷
#define COALU_RMODE_RZ			((uint32_t)0x00300000)  //舍入到0

#define COALU_FS_ENABLE			((uint32_t)0x00000000)  //fast start
#define COALU_FS_DISABLE		((uint32_t)0x00010000)  

#define COALU_LK_LINK0			((uint32_t)0x00000000)  
#define COALU_LK_LINK1			((uint32_t)0x00008000) 

/*COALU FAMW WAIT CYCLE*/
#define COALU_FAMW_WAITCYCLE2	((uint32_t)0x00004000)  
#define COALU_FAMW_WAITCYCLE1	((uint32_t)0x00000000)  //HCLK<60Hz


#define COALU_SR_DIVOFINT		((uint32_t)0x00100000)  
#define COALU_SR_DZINT			((uint32_t)0x00080000)  
#define COALU_SR_IOINT			((uint32_t)0x00040000)  
#define COALU_SR_QFINT			((uint32_t)0x00020000)  
#define COALU_SR_OFINT			((uint32_t)0x00010000)  

#define COALU_FR_FV			((uint32_t)0x00080000) 
#define COALU_FR_FZ			((uint32_t)0x00020000) 
#define COALU_FR_FN			((uint32_t)0x00010000)  
#define COALU_FR_Q			((uint32_t)0x00000010) 
#define COALU_FR_V			((uint32_t)0x00000008) 
#define COALU_FR_N			((uint32_t)0x00000001)  

#define COALU_SIMDFR_G3			((uint32_t)0x00080000)  
#define COALU_SIMDFR_G2			((uint32_t)0x00040000)  
#define COALU_SIMDFR_G1			((uint32_t)0x00020000)  
#define COALU_SIMDFR_G0			((uint32_t)0x00010000) 
#define COALU_SIMDFR_Q3			((uint32_t)0x00008000)  
#define COALU_SIMDFR_Q2			((uint32_t)0x00004000)  
#define COALU_SIMDFR_Q1			((uint32_t)0x00002000)  
#define COALU_SIMDFR_Q0			((uint32_t)0x00001000) 
#define COALU_SIMDFR_V3			((uint32_t)0x00000800)  
#define COALU_SIMDFR_V2			((uint32_t)0x00000400)  
#define COALU_SIMDFR_V1			((uint32_t)0x00000200)  
#define COALU_SIMDFR_V0			((uint32_t)0x00000100) 
#define COALU_SIMDFR_Z3			((uint32_t)0x00000080)  
#define COALU_SIMDFR_Z2			((uint32_t)0x00000040)  
#define COALU_SIMDFR_Z1			((uint32_t)0x00000020)  
#define COALU_SIMDFR_Z0			((uint32_t)0x00000010)
#define COALU_SIMDFR_N3			((uint32_t)0x00000008)  
#define COALU_SIMDFR_N2			((uint32_t)0x00000004)  
#define COALU_SIMDFR_N1			((uint32_t)0x00000002)  
#define COALU_SIMDFR_N0			((uint32_t)0x00000001)




#define COALU_ISA_OPT_Mask	 	((uint32_t)0x000003FF) 
#define COALU_ISA_OPT_SQRT	 	((uint32_t)0x00000040)  
#define COALU_ISA_OPT_LSQRT	 	((uint32_t)0x00000041)  
#define COALU_ISA_OPT_SLDIV	 	((uint32_t)0x00000080)  
#define COALU_ISA_OPT_ULDIV	 	((uint32_t)0x00000081)  
#define COALU_ISA_OPT_SADD16	((uint32_t)0x000000C0)  
#define COALU_ISA_OPT_SADD8		((uint32_t)0x000000C1) 
#define COALU_ISA_OPT_SSUB16	((uint32_t)0x000000C2) 
#define COALU_ISA_OPT_SSUB8		((uint32_t)0x000000C3) 
#define COALU_ISA_OPT_SASX		((uint32_t)0x000000C4)  
#define COALU_ISA_OPT_SSAX		((uint32_t)0x000000C5) 
#define COALU_ISA_OPT_UADD16	((uint32_t)0x000000C6) 
#define COALU_ISA_OPT_UADD8		((uint32_t)0x000000C7) 
#define COALU_ISA_OPT_USUB16  	((uint32_t)0x000000C8)  
#define COALU_ISA_OPT_USUB8		((uint32_t)0x000000C9) 
#define COALU_ISA_OPT_UASX		((uint32_t)0x000000CA) 
#define COALU_ISA_OPT_USAX		((uint32_t)0x000000CB)
#define COALU_ISA_OPT_QADD16  	((uint32_t)0x00000100)  
#define COALU_ISA_OPT_QADD8		((uint32_t)0x00000101) 
#define COALU_ISA_OPT_QSUB16	((uint32_t)0x00000102) 
#define COALU_ISA_OPT_QSUB8		((uint32_t)0x00000103)
#define COALU_ISA_OPT_QASX  	((uint32_t)0x00000104)  
#define COALU_ISA_OPT_QSAX		((uint32_t)0x00000105) 
#define COALU_ISA_OPT_UQADD16	((uint32_t)0x00000106) 
#define COALU_ISA_OPT_UQADD8	((uint32_t)0x00000107)
#define COALU_ISA_OPT_UQSUB16  	((uint32_t)0x00000108)  
#define COALU_ISA_OPT_UQSUB8	((uint32_t)0x00000109) 
#define COALU_ISA_OPT_UQASX		((uint32_t)0x0000010A) 
#define COALU_ISA_OPT_UQSAX		((uint32_t)0x0000010B)
#define COALU_ISA_OPT_QADD  	((uint32_t)0x00000141)  
#define COALU_ISA_OPT_QSUB		((uint32_t)0x00000142) 
#define COALU_ISA_OPT_QDADD		((uint32_t)0x00000143) 
#define COALU_ISA_OPT_QDSUB		((uint32_t)0x00000144)
#define COALU_ISA_OPT_SSAT16	((uint32_t)0x00000181) 
#define COALU_ISA_OPT_USAT16	((uint32_t)0x00000182)
#define COALU_ISA_OPT_SHADD16  	((uint32_t)0x000001C0)  
#define COALU_ISA_OPT_SHADD8	((uint32_t)0x000001C1) 
#define COALU_ISA_OPT_SHSUB16	((uint32_t)0x000001C2) 
#define COALU_ISA_OPT_SHSUB8	((uint32_t)0x000001C3)
#define COALU_ISA_OPT_SHASX  	((uint32_t)0x000001C4)  
#define COALU_ISA_OPT_SHSAX		((uint32_t)0x000001C5) 
#define COALU_ISA_OPT_UHADD16	((uint32_t)0x000001C6) 
#define COALU_ISA_OPT_UHADD8	((uint32_t)0x000001C7)
#define COALU_ISA_OPT_UHSUB16  	((uint32_t)0x000001C8)  
#define COALU_ISA_OPT_UHSUB8	((uint32_t)0x000001C9) 
#define COALU_ISA_OPT_UHASX		((uint32_t)0x000001CA) 
#define COALU_ISA_OPT_UHSAX		((uint32_t)0x000001CB)
#define COALU_ISA_OPT_SMLALBB  	((uint32_t)0x00000200)  
#define COALU_ISA_OPT_SMLALBT	((uint32_t)0x00000201) 
#define COALU_ISA_OPT_SMLALTB	((uint32_t)0x00000202) 
#define COALU_ISA_OPT_SMLALTT	((uint32_t)0x00000203)
#define COALU_ISA_OPT_SMLAW  	((uint32_t)0x00000204) 
#define COALU_ISA_OPT_SMLALD	((uint32_t)0x00000205) 
#define COALU_ISA_OPT_SMLALDX	((uint32_t)0x00000206) 
#define COALU_ISA_OPT_SMLSLD	((uint32_t)0x00000207)
#define COALU_ISA_OPT_SMLSLDX  	((uint32_t)0x00000208)  
#define COALU_ISA_OPT_SMMLA		((uint32_t)0x00000209) 
#define COALU_ISA_OPT_SMMLAR	((uint32_t)0x0000020A) 
#define COALU_ISA_OPT_SMMLS		((uint32_t)0x0000020B)
#define COALU_ISA_OPT_SMMLSR	((uint32_t)0x0000020C) 	
#define COALU_ISA_OPT_UMAAL		((uint32_t)0x0000020D) 
#define COALU_ISA_OPT_SMMAAA	((uint32_t)0x0000020E)
#define COALU_ISA_OPT_QSMLALBB 	((uint32_t)0x00000240)  
#define COALU_ISA_OPT_QSMLALBT	((uint32_t)0x00000241) 
#define COALU_ISA_OPT_QSMLALTB	((uint32_t)0x00000242) 
#define COALU_ISA_OPT_QSMLALTT	((uint32_t)0x00000243)
#define COALU_ISA_OPT_QSMLAW 	((uint32_t)0x00000244)  
#define COALU_ISA_OPT_QSMLALD	((uint32_t)0x00000245) 
#define COALU_ISA_OPT_QSMLALDX	((uint32_t)0x00000246)
#define COALU_ISA_OPT_QSMLSLD 	((uint32_t)0x00000247)  
#define COALU_ISA_OPT_QSMLSLDX	((uint32_t)0x00000248) 
#define COALU_ISA_OPT_QSMMLA	((uint32_t)0x00000249) 
#define COALU_ISA_OPT_QSMMLAR	((uint32_t)0x0000024A)
#define COALU_ISA_OPT_QSMMLS 	((uint32_t)0x0000024B)  
#define COALU_ISA_OPT_QSMMLSR	((uint32_t)0x0000024C) 
#define COALU_ISA_OPT_QUMAAL	((uint32_t)0x0000024D) 
#define COALU_ISA_OPT_QSMMAAA	((uint32_t)0x0000024E)
#define COALU_ISA_OPT_VADDF32	((uint32_t)0x00000280)  
#define COALU_ISA_OPT_VSUBF32	((uint32_t)0x00000281) 
#define COALU_ISA_OPT_VMULF32	((uint32_t)0x00000282) 
#define COALU_ISA_OPT_VDIVF32	((uint32_t)0x00000283) 
#define COALU_ISA_OPT_VSQRTF32		((uint32_t)0x00000284) 
#define COALU_ISA_OPT_VLSQRTF32		((uint32_t)0x00000285) 
#define COALU_ISA_OPT_VCVTRS32F32	((uint32_t)0x00000289)
#define COALU_ISA_OPT_VCVTF32S32	((uint32_t)0x0000028A)
#define COALU_ISA_OPT_VMLAF32		((uint32_t)0x0000028F) 
#define COALU_ISA_OPT_VMLSF32		((uint32_t)0x00000290) 
#define COALU_ISA_OPT_VNMLAF32		((uint32_t)0x00000292)
#define COALU_ISA_OPT_VNMLSF32		((uint32_t)0x00000293)
#define COALU_ISA_OPT_VNMUL			((uint32_t)0x00000294) 
#define COALU_ISA_OPT_VCVTRS64F32 	((uint32_t)0x00000295)
#define COALU_ISA_OPT_VCVTF32S64	((uint32_t)0x00000296)


typedef struct	
{
  uint32_t COALU_RMode;	
//  uint32_t COALU_FSMode;		//Fast start enable/disable
  uint32_t COALU_Link;
  uint32_t COALU_FAMW;			//wait cycle   
  uint32_t COALU_Inter;			//interrupt enable/disable
}COALU_InitTypeDef;


extern void COALU_ITConfig(uint32_t COALU_IT, FunctionalState NewState);
extern void coalu_enable(void);
CMSIS_INLINE uint32_t __SQRT(uint32_t x);
CMSIS_INLINE uint32_t __LSQRT(uint64_t x);
CMSIS_INLINE uint64_t __SLDIV(int64_t x, int32_t y, int32_t *remainder);
CMSIS_INLINE uint64_t __ULDIV(uint64_t x, uint32_t y, uint32_t *remainder);
CMSIS_INLINE uint32_t __SADD16(uint32_t x, uint32_t y);
CMSIS_INLINE uint32_t __SADD8(uint32_t x, uint32_t y);
CMSIS_INLINE uint32_t __SSUB16(uint32_t x, uint32_t y);
CMSIS_INLINE uint32_t __SSUB8(uint32_t x, uint32_t y);
CMSIS_INLINE uint32_t __SASX(uint32_t OPT0, uint32_t OPT1);
CMSIS_INLINE uint32_t __SSAX(uint32_t OPT0, uint32_t OPT1);
CMSIS_INLINE uint32_t __UADD16(uint32_t x, uint32_t y);
CMSIS_INLINE uint32_t __UADD8(uint32_t x, uint32_t y);
CMSIS_INLINE uint32_t __USUB16(uint32_t x, uint32_t y);
CMSIS_INLINE uint32_t __USUB8(uint32_t x, uint32_t y);
CMSIS_INLINE uint32_t __UASX(uint32_t OPT0, uint32_t OPT1);
CMSIS_INLINE uint32_t __USAX(uint32_t OPT0, uint32_t OPT1);
CMSIS_INLINE uint32_t __QADD16(uint32_t x, uint32_t y);
CMSIS_INLINE uint32_t __QADD8(uint32_t x, uint32_t y);
CMSIS_INLINE uint32_t __QSUB16(uint32_t x, uint32_t y);
CMSIS_INLINE uint32_t __QSUB8(uint32_t x, uint32_t y);
CMSIS_INLINE uint32_t __QASX(uint32_t x, uint32_t y);
CMSIS_INLINE uint32_t __QSAX(uint32_t x, uint32_t y);
CMSIS_INLINE uint32_t __UQADD16(uint32_t x, uint32_t y);
CMSIS_INLINE uint32_t __UQADD8(uint32_t x, uint32_t y);
CMSIS_INLINE uint32_t __UQSUB16(uint32_t x, uint32_t y);
CMSIS_INLINE uint32_t __UQSUB8(uint32_t x, uint32_t y);
CMSIS_INLINE uint32_t __UQASX(uint32_t OPT0, uint32_t OPT1);
CMSIS_INLINE uint32_t __UQSAX(uint32_t OPT0, uint32_t OPT1);
CMSIS_INLINE int32_t __QADD(int32_t x, int32_t y);
CMSIS_INLINE int32_t __QSUB(int32_t x, int32_t y);
CMSIS_INLINE uint32_t __QDADD(uint32_t x, uint32_t y);
CMSIS_INLINE uint32_t __QDSUB(uint32_t x, uint32_t y);
CMSIS_INLINE uint32_t __SSAT16(uint32_t x, uint8_t n);
CMSIS_INLINE uint32_t __USAT16(uint32_t x, uint8_t n);
CMSIS_INLINE uint32_t __SHADD16(uint32_t x, uint32_t y);
CMSIS_INLINE uint32_t __SHADD8(uint32_t x, uint32_t y);
CMSIS_INLINE uint32_t __SHSUB16(uint32_t x, uint32_t y);
CMSIS_INLINE uint32_t __SHSUB8(uint32_t x, uint32_t y);
CMSIS_INLINE uint32_t __SHASX(uint32_t OPT0, uint32_t OPT1);
CMSIS_INLINE uint32_t __SHSAX(uint32_t OPT0, uint32_t OPT1);
CMSIS_INLINE uint32_t __UHADD16(uint32_t x, uint32_t y);
CMSIS_INLINE uint32_t __UHADD8(uint32_t x, uint32_t y);
CMSIS_INLINE uint32_t __UHSUB16(uint32_t x, uint32_t y);
CMSIS_INLINE uint32_t __UHSUB8(uint32_t x, uint32_t y);
CMSIS_INLINE uint32_t __UHASX(uint32_t OPT0, uint32_t OPT1);
CMSIS_INLINE uint32_t __UHSAX(uint32_t OPT0, uint32_t OPT1);
CMSIS_INLINE int64_t __SMLALBB(uint32_t x, uint32_t y, int64_t z);
CMSIS_INLINE int64_t __SMLALBT(uint32_t x, uint32_t y, int64_t z);
CMSIS_INLINE int64_t __SMLALTB(uint32_t x, uint32_t y, int64_t z);
CMSIS_INLINE int64_t __SMLALTT(uint32_t x, uint32_t y, int64_t z);
CMSIS_INLINE int32_t __SMLAW(int32_t x, int32_t y, int32_t z);
CMSIS_INLINE uint64_t __SMLALD(uint32_t x, uint32_t y, uint64_t z);
CMSIS_INLINE uint64_t __SMLALDX(uint32_t x, uint32_t y, uint64_t z);
CMSIS_INLINE int64_t __SMLSLD(uint32_t x, uint32_t y, int64_t z);
CMSIS_INLINE int64_t __SMLSLDX(uint32_t x, uint32_t y, int64_t z);
CMSIS_INLINE int32_t __SMMLA(int32_t x, int32_t y, int32_t z);
CMSIS_INLINE int32_t __SMMLAR(int32_t x, int32_t y, int32_t z);
CMSIS_INLINE int32_t __SMMLS(int32_t x, int32_t y, int32_t z);
CMSIS_INLINE int32_t __SMMLSR(int32_t x, int32_t y, int32_t z);
CMSIS_INLINE int64_t __UMAAL(int32_t x, int32_t y, int64_t z);
CMSIS_INLINE int32_t __SMMAAA(int32_t x1, int32_t x2, int32_t y1, int32_t y2, int32_t z);
CMSIS_INLINE int64_t __QSMLALBB(uint32_t x, uint32_t y, int64_t z);
CMSIS_INLINE int64_t __QSMLALBT(uint32_t x, uint32_t y, int64_t z);
CMSIS_INLINE int64_t __QSMLALTB(uint32_t x, uint32_t y, int64_t z);
CMSIS_INLINE int64_t __QSMLALTT(uint32_t x, uint32_t y, int64_t z);
CMSIS_INLINE int32_t __QSMLAW(int32_t x, int32_t y, int32_t z);
CMSIS_INLINE int64_t __QSMLALD(uint32_t x, uint32_t y, int64_t z);
CMSIS_INLINE int64_t __QSMLALDX(uint32_t x, uint32_t y, int64_t z);
CMSIS_INLINE int64_t __QSMLSLD(uint32_t x, uint32_t y, int64_t z);
CMSIS_INLINE int64_t __QSMLSLDX(uint32_t x, uint32_t y, int64_t z);
CMSIS_INLINE int32_t __QSMMLA(int32_t x, int32_t y, int32_t z);
CMSIS_INLINE int32_t __QSMMLAR(int32_t x, int32_t y, int32_t z);
CMSIS_INLINE int32_t __QSMMLS(int32_t x, int32_t y, int32_t z);
CMSIS_INLINE int32_t __QSMMLSR(int32_t x, int32_t y, int32_t z);
CMSIS_INLINE int64_t __QUMAAL(uint32_t x, uint32_t y, int64_t z);
CMSIS_INLINE int32_t __QSMMAAA(int32_t x1, int32_t x2, int32_t y1, int32_t y2, int32_t z);


/**********************floating point ********************************/
#ifdef USE_HK_FLOAT_POINT_FUNC	
CMSIS_INLINE float32_t __VADDF32(float32_t x, float32_t y);
#else
CMSIS_INLINE float32_t __aeabi_fadd(float32_t x, float32_t y);
#endif

#ifdef USE_HK_FLOAT_POINT_FUNC	
CMSIS_INLINE float32_t __VSUBF32(float32_t x, float32_t y);
#else
CMSIS_INLINE float32_t __aeabi_fsub(float32_t x, float32_t y);
#endif

#ifdef USE_HK_FLOAT_POINT_FUNC	
CMSIS_INLINE float32_t __VMULF32(float32_t x, float32_t y);
#else
CMSIS_INLINE float32_t __aeabi_fmul(float32_t x, float32_t y);
#endif

#ifdef USE_HK_FLOAT_POINT_FUNC	
CMSIS_INLINE float32_t __VDIVF32(float32_t x, float32_t y);
#else
CMSIS_INLINE float32_t __aeabi_fdiv(float32_t x, float32_t y);
#endif

#ifdef USE_HK_FLOAT_POINT_FUNC	
CMSIS_INLINE float32_t __VSQRTF32(float32_t x);
#else
CMSIS_INLINE float32_t sqrtf(float32_t x);
#endif

CMSIS_INLINE float32_t __VLSQRTF32(float32_t x);
CMSIS_INLINE int32_t __VCVTRS32F32(float32_t x, uint32_t RMODE);

#ifdef USE_HK_FLOAT_POINT_FUNC	
CMSIS_INLINE int32_t __VCVTRS32F32(float32_t x);
#else
CMSIS_INLINE int32_t __aeabi_f2iz(float32_t x);
#endif

#ifdef USE_HK_FLOAT_POINT_FUNC	
CMSIS_INLINE int64_t __VCVTRS64F32(float32_t x);
#else
CMSIS_INLINE int64_t __aeabi_f2lz(float32_t x);
#endif

//CMSIS_INLINE int32_t __aeabi_f2iz(float32_t x);
//CMSIS_INLINE float32_t __aeabi_i2f(int32_t x);
CMSIS_INLINE float32_t __VMLAF32(float32_t x, float32_t y,float32_t z );
CMSIS_INLINE float32_t __VMLSF32(float32_t x, float32_t y,float32_t z );
CMSIS_INLINE float32_t __VNMLAF32(float32_t x, float32_t y,float32_t z );
CMSIS_INLINE float32_t __VNMLSF32(float32_t x, float32_t y,float32_t z );
CMSIS_INLINE float32_t __VNMUL(float32_t x, float32_t y);
CMSIS_INLINE float32_t __VCVTF32S64(int64_t x);
CMSIS_INLINE float32_t __VCVTF32S32(int32_t x);


#ifdef __cplusplus
}
#endif

#endif /* __HK32F39A_COALU_H */


