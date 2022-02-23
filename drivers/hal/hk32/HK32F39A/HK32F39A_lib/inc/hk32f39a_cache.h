/**
  ******************************************************************************
  * @file    hk32f39a_cache.h
  * @author  Thomas.W
  * @version V1.0  
  * @brief   Header file of cache module
  * @changelist  
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HK32F39A_CACHE_H
#define __HK32F39A_CACHE_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "hk32f39a.h"

void Cache_ReadHITCNTRegister(uint32_t *HIT_CNT);
void Cache_ReadMISSCNTRegister(uint32_t *MISS_CNT);
void Cache_ServeCTL(uint32_t CacheServeCtrl);
void CACHE_IVTCacheConfig(FunctionalState NewState);
void CACHE_ProgFlushConfig(FunctionalState NewState);
void CACHE_ForceFlushConfig(FunctionalState NewState);
void CACHE_HitMissCounterConfig(FunctionalState NewState);				
void CACHE_HitMissCounterClear(FunctionalState NewState);

#ifdef __cplusplus
}
#endif

#endif 
