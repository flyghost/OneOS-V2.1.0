/**
  ******************************************************************************
  * @file    hk32f39a_coalu.c
  * @author  laura.c    
  * @version V1.0.0
  * @date    2019-10-18
  * @brief   This file provides all the CO_ALU firmware functions.
  ****************************************************************************** 
  */

/* Includes ------------------------------------------------------------------*/
#include "hk32f39a.h"
#include "hk32f39a_coalu.h"

void coalu_enable(void)
{
	RCC->CFGR6|=RCC_AHBPeriph2_COALU;
	RCC_AHBPeriph2ClockCmd(RCC_AHBPeriph2_COALU,ENABLE);

}

//result=x+y
#ifdef USE_HK_FLOAT_POINT_FUNC	
CMSIS_INLINE float32_t __VADDF32(float32_t x, float32_t y)
#else
CMSIS_INLINE float32_t __aeabi_fadd(float32_t x, float32_t y)
#endif
{
	//start opt 
	COALU->CR = COALU_FAMW_WAITCYCLE2|COALU_ISA_OPT_VADDF32;
	COALU->R1.f = y;
	COALU->R0.f = x;
#ifdef COALU_FSMode_DISABLE
	COALU->CR|=COALU_START;
#endif

	return (float32_t)(COALU->R5.f);
}


//result=x-y
#ifdef USE_HK_FLOAT_POINT_FUNC	
CMSIS_INLINE float32_t __VSUBF32(float32_t x, float32_t y)
#else
CMSIS_INLINE float32_t __aeabi_fsub(float32_t x, float32_t y)
#endif
{
	//start opt 
	COALU->CR = COALU_FAMW_WAITCYCLE2|COALU_ISA_OPT_VSUBF32;
	COALU->R1.f = x;
	COALU->R0.f = y;
#ifdef COALU_FSMode_DISABLE
	COALU->CR|=COALU_START;
#endif

	return (COALU->R5.f);
}

//result=x*y
#ifdef USE_HK_FLOAT_POINT_FUNC	
CMSIS_INLINE float32_t __VMULF32(float32_t x, float32_t y)
#else
CMSIS_INLINE float32_t __aeabi_fmul(float32_t x, float32_t y)
#endif
{
	//start opt 
	COALU->CR = COALU_FAMW_WAITCYCLE2|COALU_ISA_OPT_VMULF32;
	COALU->R1.f = x;
	COALU->R0.f = y;
#ifdef COALU_FSMode_DISABLE
	COALU->CR|=COALU_START;
#endif

	return (COALU->R5.f);
}

//result=x/y
#ifdef USE_HK_FLOAT_POINT_FUNC	
CMSIS_INLINE float32_t __VDIVF32(float32_t x, float32_t y)
#else
CMSIS_INLINE float32_t __aeabi_fdiv(float32_t x, float32_t y)
#endif
{
	//start opt 
	COALU->CR = COALU_FAMW_WAITCYCLE2|COALU_ISA_OPT_VDIVF32;
	COALU->R1.f = x;
	COALU->R0.f = y;
#ifdef COALU_FSMode_DISABLE
	COALU->CR|=COALU_START;
#endif

	return (COALU->R5.f);
}


#ifdef USE_HK_FLOAT_POINT_FUNC	
CMSIS_INLINE float32_t __VSQRTF32(float32_t x)
#else
CMSIS_INLINE float32_t sqrtf(float32_t x)
#endif
{
	//start opt 
	COALU->CR = COALU_FAMW_WAITCYCLE2|COALU_ISA_OPT_VSQRTF32;
	COALU->R0.f = x;
#ifdef COALU_FSMode_DISABLE
	COALU->CR|=COALU_START;
#endif

	return (COALU->R5.f);
}

#ifdef USE_HK_FLOAT_POINT_FUNC	
CMSIS_INLINE int32_t __VCVTRS32F32(float32_t x)
#else
CMSIS_INLINE int32_t __aeabi_f2iz(float32_t x)
#endif
{
	//start opt 
	COALU->CR = COALU_FAMW_WAITCYCLE2|COALU_ISA_OPT_VCVTRS32F32|COALU_RMODE_RZ;//COALU_FAMW_WAITCYCLE2|COALU_ISA_OPT_VCVTRS32F32|RMODE;
	COALU->R0.f = x;
#ifdef COALU_FSMode_DISABLE
	COALU->CR|=COALU_START;
#endif

	return (COALU->R5.u);
}
//把浮点数转换为64位带符号定点数
#ifdef USE_HK_FLOAT_POINT_FUNC	
CMSIS_INLINE int64_t __VCVTRS64F32(float32_t x)
#else
CMSIS_INLINE int64_t __aeabi_f2lz(float32_t x)
#endif
{
	//start opt 
	COALU->CR = COALU_FAMW_WAITCYCLE2|COALU_ISA_OPT_VCVTRS64F32|COALU_RMODE_RZ;
	COALU->R0.f = x;
#ifdef COALU_FSMode_DISABLE
	COALU->CR|=COALU_START;
#endif

	return ((int64_t)(COALU->R5.u)|((uint64_t)(COALU->R6.u) <<32));
}

//高精度浮点开方
CMSIS_INLINE float32_t __VLSQRTF32(float32_t x)
{
	//start opt 
	COALU->CR = COALU_FAMW_WAITCYCLE2|COALU_ISA_OPT_VLSQRTF32;
	COALU->R0.f = x;
#ifdef COALU_FSMode_DISABLE
	COALU->CR|=COALU_START;
#endif

	return (COALU->R5.f);
}


//把32位带符号定点数转换为浮点数
CMSIS_INLINE float32_t __VCVTF32S32(int32_t x)
{
	//start opt 
	COALU->CR = COALU_FAMW_WAITCYCLE2|COALU_ISA_OPT_VCVTF32S32;
	COALU->R0.u = x;
#ifdef COALU_FSMode_DISABLE
	COALU->CR|=COALU_START;
#endif

	return (COALU->R5.f);
}

//result=z+x*y
CMSIS_INLINE float32_t __VMLAF32(float32_t x, float32_t y,float32_t z )
{
	//start opt 
	COALU->CR = COALU_FAMW_WAITCYCLE2|COALU_ISA_OPT_VMLAF32;
	COALU->R2.f = z;	
	COALU->R1.f = y;
	COALU->R0.f = x;
#ifdef COALU_FSMode_DISABLE
	COALU->CR|=COALU_START;
#endif

	return (COALU->R5.f);
}

//result=z-x*y
CMSIS_INLINE float32_t __VMLSF32(float32_t x, float32_t y,float32_t z )
{
	//start opt 
	COALU->CR = COALU_FAMW_WAITCYCLE2|COALU_ISA_OPT_VMLSF32;
	COALU->R2.f = z;	
	COALU->R1.f = y;
	COALU->R0.f = x;
#ifdef COALU_FSMode_DISABLE
	COALU->CR|=COALU_START;
#endif

	return (COALU->R5.f);
}

//result=-(z+x*y)
CMSIS_INLINE float32_t __VNMLAF32(float32_t x, float32_t y,float32_t z )
{
	//start opt 
	COALU->CR = COALU_FAMW_WAITCYCLE2|COALU_ISA_OPT_VNMLAF32;
	COALU->R2.f = z;	
	COALU->R1.f = y;
	COALU->R0.f = x;
#ifdef COALU_FSMode_DISABLE
	COALU->CR|=COALU_START;
#endif

	return (COALU->R5.f);
}

//result=-(z-x*y)
CMSIS_INLINE float32_t __VNMLSF32(float32_t x, float32_t y,float32_t z )
{
	//start opt 
	COALU->CR = COALU_FAMW_WAITCYCLE2|COALU_ISA_OPT_VNMLSF32;
	COALU->R2.f = z;	
	COALU->R1.f = y;
	COALU->R0.f = x;
#ifdef COALU_FSMode_DISABLE
	COALU->CR|=COALU_START;
#endif

	return (COALU->R5.f);
}

//result=-x*y
CMSIS_INLINE float32_t __VNMUL(float32_t x, float32_t y)
{
	//start opt 
	COALU->CR = COALU_FAMW_WAITCYCLE2|COALU_ISA_OPT_VNMUL;
	COALU->R1.f = x;
	COALU->R0.f = y;
#ifdef COALU_FSMode_DISABLE
	COALU->CR|=COALU_START;
#endif

	return (COALU->R5.f);
}

//把64位带符号定点数转换为浮点数
CMSIS_INLINE float32_t __VCVTF32S64(int64_t x)
{
	//start opt 
	COALU->CR = COALU_FAMW_WAITCYCLE2|COALU_ISA_OPT_VCVTF32S64;
	COALU->R1.u = (uint32_t)(x>>32);
	COALU->R0.u = (uint32_t)(x&0xffffffff);

#ifdef COALU_FSMode_DISABLE
	COALU->CR|=COALU_START;
#endif

	return (COALU->R5.f);
}

