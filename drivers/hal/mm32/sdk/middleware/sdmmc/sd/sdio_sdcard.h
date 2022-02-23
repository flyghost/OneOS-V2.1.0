////////////////////////////////////////////////////////////////////////////////
/// @file    sdio_sdcard.h
/// @author  AE TEAM
/// @brief   THIS FILE PROVIDES ALL THE SYSTEM FIRMWARE FUNCTIONS.
////////////////////////////////////////////////////////////////////////////////
/// @attention
///
/// THE EXISTING FIRMWARE IS ONLY FOR REFERENCE, WHICH IS DESIGNED TO PROVIDE
/// CUSTOMERS WITH CODING INFORMATION ABOUT THEIR PRODUCTS SO THEY CAN SAVE
/// TIME. THEREFORE, MINDMOTION SHALL NOT BE LIABLE FOR ANY DIRECT, INDIRECT OR
/// CONSEQUENTIAL DAMAGES ABOUT ANY CLAIMS ARISING OUT OF THE CONTENT OF SUCH
/// HARDWARE AND/OR THE USE OF THE CODING INFORMATION CONTAINED HEREIN IN
/// CONNECTION WITH PRODUCTS MADE BY CUSTOMERS.
///
/// <H2><CENTER>&COPY; COPYRIGHT MINDMOTION </CENTER></H2>
////////////////////////////////////////////////////////////////////////////////

// Define to prevent recursive inclusion
#ifndef __SDIIO_SDCARD_H
#define __SDIIO_SDCARD_H

// Files includes
#include "HAL_conf.h"
#include "HAL_sdio.h"

////////////////////////////////////////////////////////////////////////////////
/// @defgroup MM32_Example_Layer
/// @brief MM32 Example Layer
/// @{

////////////////////////////////////////////////////////////////////////////////
/// @defgroup MM32_RESOURCE
/// @brief MM32 Examples resource modules
/// @{

////////////////////////////////////////////////////////////////////////////////
/// @defgroup MM32_Exported_Constants
/// @{


/// @}

////////////////////////////////////////////////////////////////////////////////
/// @defgroup MM32_Exported_Enumeration
/// @{

////////////////////////////////////////////////////////////////////////////////
/// @brief XXXX enumerate definition.
/// @anchor XXXX
////////////////////////////////////////////////////////////////////////////////



/// @}

////////////////////////////////////////////////////////////////////////////////
/// @defgroup MM32_Exported_Variables
/// @{
#ifdef _SDIIO_SDCARD_C_
#define GLOBAL

#else
#define GLOBAL extern

extern u8 CardType;





#endif





#undef GLOBAL

/// @}


////////////////////////////////////////////////////////////////////////////////
/// @defgroup MM32_Exported_Functions
/// @{




SD_Error SD_EnableWideBusOperation(u32 wmode);
SD_Error SDEnWideBus(u8 enx);
SD_Error SD_Init(void);
SD_Error SD_PowerON(void);
SD_Error SD_InitializeCards(void);
SD_Error SD_GetCardInfo(SD_CardInfo* cardinfo);
//SD_Error SD_SetDeviceMode(u32 mode);
SD_Error SD_SelectDeselect(u32 addr);
SD_Error SD_SendStatus(u32* pcardstatus);
SDCardState SD_GetState(void);
SD_Error SD_ReadOneBlockPolling(u8* buf, long long  addr, u16 blksize);
SD_Error SD_WriteOneBlockPolling(const u8* buf, long long addr, u16 blksize);
SD_Error SD_ProcessIRQSrc(void);
SD_Error IsCardProgramming(u8* pstatus);

u8 SD_ReadDisk(u8* buf, u32 sector, u32 cnt);
u8 SD_WriteDisk(const u8* buf, u32 sector, u32 cnt);

/// @}


/// @}

/// @}


////////////////////////////////////////////////////////////////////////////////
#endif
////////////////////////////////////////////////////////////////////////////////



