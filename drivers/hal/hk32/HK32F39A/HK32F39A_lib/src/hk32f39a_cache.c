/**
  ******************************************************************************
  * @file    hk32f39a_cache.c
  * @author  Thomas.W
  * @version V1.0  
  * @brief   API file of cache module
  * @changelist  
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "hk32f39a_cache.h"
#include "hk32f39a_rcc.h"

/* ------------ Cache Control registers bit address in the alias region --------------- */
#define CACHE_OFFSET     (CACHE_BASE - PERIPH_BASE)

/* --- Cache CTL Register ---*/
#define CACHE_CTL_OFFSET      		(CACHE_OFFSET)
#define IV_CACHE_EN_BitNumber    	0x08
#define PE_FLUSH_EN_BitNumber   	0x0A
#define FORCE_FLUSH_BitNumber   	0x0B
#define PR_CNT_EN_BitNumber   		0x0C
#define PR_CNT_CLR_BitNumber   		0x0D
#define CACHECTL_IV_CACHE_EN_BB   (PERIPH_BB_BASE + (CACHE_CTL_OFFSET * 32) + (IV_CACHE_EN_BitNumber * 4))
#define CACHECTL_PE_FLUSH_EN_BB   (PERIPH_BB_BASE + (CACHE_CTL_OFFSET * 32) + (PE_FLUSH_EN_BitNumber * 4))
#define CACHECTL_FORCE_FLUSH_BB   (PERIPH_BB_BASE + (CACHE_CTL_OFFSET * 32) + (FORCE_FLUSH_BitNumber * 4))
#define CACHECTL_PR_CNT_EN_BB     (PERIPH_BB_BASE + (CACHE_CTL_OFFSET * 32) + (PR_CNT_EN_BitNumber * 4))
#define CACHECTL_PR_CNT_CLR_BB    (PERIPH_BB_BASE + (CACHE_CTL_OFFSET * 32) + (PR_CNT_CLR_BitNumber * 4))

#define CACHE_CTL_MSK   	0x00000007
#define CACHE_CTL_None   	0x00000000
#define CACHE_CTL_IBUS   	0x00000001
#define CACHE_CTL_FSMC   	0x00000002
#define CACHE_CTL_QSPI   	0x00000004

/**
  * @brief  Enables or disables the Interrrupt vector table cache function.
  * @param  FunctionalState: 					
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void CACHE_IVTCacheConfig(FunctionalState NewState)
{
  *(__IO uint32_t *) CACHECTL_IV_CACHE_EN_BB = (uint32_t)NewState;    
}

/**
  * @brief  Clear Cache content when Flash on chip is programmed or erased.
  * @param  FunctionalState: This parameter can be: ENABLE or DISABLE
  * ENABLE: Clear Cache content; DISABLE:Don't clear Cache content
  * @retval None
  */
void CACHE_ProgFlushConfig(FunctionalState NewState)
{
  *(__IO uint32_t *) CACHECTL_PE_FLUSH_EN_BB = (uint32_t)NewState;    
}

/**
  * @brief  Force to Clear Cache content immediately.
  * @param  FunctionalState: This parameter can be: ENABLE or DISABLE
  * ENABLE: Clear Cache content; DISABLE:Don't clear Cache content
  * @retval None
  */
void CACHE_ForceFlushConfig(FunctionalState NewState)
{
  *(__IO uint32_t *) CACHECTL_FORCE_FLUSH_BB = (uint32_t)NewState;    
}

/**
  * @brief  Enable or Disable Cache hit/miss counter to continue counting.
  * @param  FunctionalState: This parameter can be: ENABLE or DISABLE
  * ENABLE: Hit and miss counter start working; 
  * DISABLE:Hit and miss counter stop counting;
  * @retval None
  */
void CACHE_HitMissCounterConfig(FunctionalState NewState)
{
  *(__IO uint32_t *) CACHECTL_PR_CNT_EN_BB = (uint32_t)NewState;    
}

/**
  * @brief  Clear Cache hit/miss counter.
  * @param  FunctionalState: This parameter can be: ENABLE or DISABLE
  * ENABLE: Clear Hit and miss counter; 
  * DISABLE:No Action;
  * @retval None
  */
void CACHE_HitMissCounterClear(FunctionalState NewState)
{
  *(__IO uint32_t *) CACHECTL_PR_CNT_CLR_BB = (uint32_t)NewState;    
}

/**
  * @brief  Reads data from HIT_CNT_H and HIT_CNT_H Register.
  * @param  HIT_CNT: specifies the Data pointer.
  *   HIT_CNT[0] is HIT_CNT_L, HIT_CNT[1] is HIT_CNT_H
  * @retval None
  */
void Cache_ReadHITCNTRegister(uint32_t *HIT_CNT)
{
		HIT_CNT[0] = CACHE->HIT_CNT_L;
		HIT_CNT[1] = CACHE->HIT_CNT_H;
}

/**
  * @brief  Reads data from MISS_CNT_H and MISS_CNT_H Register.
  * @param  HIT_CNT: specifies the Data pointer.
  *   MISS_CNT[0] is MISS_CNT_L, MISS_CNT[1] is MISS_CNT_H
  * @retval None
  */
void Cache_ReadMISSCNTRegister(uint32_t *MISS_CNT)
{
		MISS_CNT[0] = CACHE->MISS_CNT_L;
		MISS_CNT[1] = CACHE->MISS_CNT_H;
}

/**
  * @brief  Config Cache function to serve for I-Bus or FSMC or QSPI.
  * @param  CACHE_CTL_None: Disable Cache, don't serve for I-Bus or FSMC or QSPI.
  					CACHE_CTL_IBUS: Enable Cache and Serve for I-bus.
						CACHE_CTL_FSMC: Enable Cache and Serve for FSMC.
						CACHE_CTL_QSPI: Enable Cache and Serve for QSPI.																						
  * @retval None
  */
void Cache_ServeCTL(uint32_t CacheServeCtrl)
{
		CACHE->CTL &= (~CACHE_CTL_MSK);
		CACHE->CTL |= CacheServeCtrl;
}

