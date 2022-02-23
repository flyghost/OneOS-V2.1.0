/******************************************************************************
* Copyright (C) 2017, Huada Semiconductor Co.,Ltd All rights reserved.    
*
* This software is owned and published by: 
* Huada Semiconductor Co.,Ltd ("HDSC").
*
* BY DOWNLOADING, INSTALLING OR USING THIS SOFTWARE, YOU AGREE TO BE BOUND 
* BY ALL THE TERMS AND CONDITIONS OF THIS AGREEMENT.
*
* This software contains source code for use with HDSC 
* components. This software is licensed by HDSC to be adapted only 
* for use in systems utilizing HDSC components. HDSC shall not be 
* responsible for misuse or illegal use of this software for devices not 
* supported herein. HDSC is providing this software "AS IS" and will 
* not be responsible for issues arising from incorrect user implementation 
* of the software.  
*
* Disclaimer:
* HDSC MAKES NO WARRANTY, EXPRESS OR IMPLIED, ARISING BY LAW OR OTHERWISE,
* REGARDING THE SOFTWARE (INCLUDING ANY ACOOMPANYING WRITTEN MATERIALS), 
* ITS PERFORMANCE OR SUITABILITY FOR YOUR INTENDED USE, INCLUDING, 
* WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, THE IMPLIED 
* WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE OR USE, AND THE IMPLIED 
* WARRANTY OF NONINFRINGEMENT.  
* HDSC SHALL HAVE NO LIABILITY (WHETHER IN CONTRACT, WARRANTY, TORT, 
* NEGLIGENCE OR OTHERWISE) FOR ANY DAMAGES WHATSOEVER (INCLUDING, WITHOUT 
* LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, 
* LOSS OF BUSINESS INFORMATION, OR OTHER PECUNIARY LOSS) ARISING FROM USE OR 
* INABILITY TO USE THE SOFTWARE, INCLUDING, WITHOUT LIMITATION, ANY DIRECT, 
* INDIRECT, INCIDENTAL, SPECIAL OR CONSEQUENTIAL DAMAGES OR LOSS OF DATA, 
* SAVINGS OR PROFITS, 
* EVEN IF Disclaimer HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES. 
* YOU ASSUME ALL RESPONSIBILITIES FOR SELECTION OF THE SOFTWARE TO ACHIEVE YOUR
* INTENDED RESULTS, AND FOR THE INSTALLATION OF, USE OF, AND RESULTS OBTAINED 
* FROM, THE SOFTWARE.  
*
* This software may be replicated in part or whole for the licensed use, 
* with the restriction that this Disclaimer and Copyright notice must be 
* included with each copy of this software, whether used in part or whole, 
* at all times.
*/
/******************************************************************************/
/** \file evbhc32lf07x.h
 **
 ** stk board common define.
 ** @link SampleGroup Some description @endlink
 **
 **   - 2018-03-09  1.0  Lux First version.
 **
 ******************************************************************************/
#ifndef __BOARD_EVBHC32LF07X_H__
#define __BOARD_EVBHC32LF07X_H__

///< #define BOARD_EVBHC32LF07X_V10
#define BOARD_EVBHC32LF07X_V11




///< EVB GPIO DEFINE
///< USER KEY1
#define EVB_KEY1_PORT       GpioPortC
#define EVB_KEY1_PIN        GpioPin12
///< USER KEY2
#define EVB_KEY2_PORT       GpioPortD
#define EVB_KEY2_PIN        GpioPin3
///< USER KEY3
#define EVB_KEY3_PORT       GpioPortD
#define EVB_KEY3_PIN        GpioPin2
///< USER KEY4
#define EVB_KEY4_PORT       GpioPortD
#define EVB_KEY4_PIN        GpioPin4

#ifdef BOARD_EVBHC32LF07X_V11
///< USER KEY5
#define EVB_KEY5_PORT       GpioPortD
#define EVB_KEY5_PIN        GpioPin6
#else
///< USER KEY5
#define EVB_KEY5_PORT       GpioPortA
#define EVB_KEY5_PIN        GpioPin15    
#endif

///< LED RED
#define EVB_LEDR_PORT        GpioPortE
#define EVB_LEDR_PIN         GpioPin3
///< LED YELLOW
#define EVB_LEDY_PORT        GpioPortE
#define EVB_LEDY_PIN         GpioPin2

#ifdef BOARD_EVBHC32LF07X_V11
///< LED GREEN
#define EVB_LEDG_PORT        GpioPortE
#define EVB_LEDG_PIN         GpioPin0
///< LED BLUE
#define EVB_LEDB_PORT        GpioPortE
#define EVB_LEDB_PIN         GpioPin1
#else
///< LED GREEN
#define EVB_LEDG_PORT        GpioPortB
#define EVB_LEDG_PIN         GpioPin9
///< LED BLUE
#define EVB_LEDB_PORT        GpioPortB
#define EVB_LEDB_PIN         GpioPin8
#endif

    
///< XTH
#define SYSTEM_XTH          (8*1000*1000u)     ///< 8MHz

#define EVB_XTHI_PORT       GpioPortF
#define EVB_XTHI_PIN        GpioPin0
#define EVB_XTHO_PORT       GpioPortF
#define EVB_XTHO_PIN        GpioPin1

///< XTL
#define SYSTEM_XTL          (32768u)            ///< 32768Hz
#define EVB_XTLI_PORT       GpioPortC
#define EVB_XTLI_PIN        GpioPin14
#define EVB_XTLO_PORT       GpioPortC
#define EVB_XTLO_PIN        GpioPin15

///< LCD
#define EVB_LCD_COM0_PORT   GpioPortA
#define EVB_LCD_COM0_PIN    GpioPin9
#define EVB_LCD_COM1_PORT   GpioPortA
#define EVB_LCD_COM1_PIN    GpioPin10
#define EVB_LCD_COM2_PORT   GpioPortF
#define EVB_LCD_COM2_PIN    GpioPin6
#define EVB_LCD_COM3_PORT   GpioPortA
#define EVB_LCD_COM3_PIN    GpioPin15

#define EVB_LCD_SEG0_PORT   GpioPortA
#define EVB_LCD_SEG0_PIN    GpioPin8
#define EVB_LCD_SEG1_PORT   GpioPortC
#define EVB_LCD_SEG1_PIN    GpioPin9
#define EVB_LCD_SEG2_PORT   GpioPortC
#define EVB_LCD_SEG2_PIN    GpioPin8
#define EVB_LCD_SEG3_PORT   GpioPortC
#define EVB_LCD_SEG3_PIN    GpioPin7
#define EVB_LCD_SEG4_PORT   GpioPortC
#define EVB_LCD_SEG4_PIN    GpioPin6
#define EVB_LCD_SEG5_PORT   GpioPortB
#define EVB_LCD_SEG5_PIN    GpioPin15
#define EVB_LCD_SEG6_PORT   GpioPortB
#define EVB_LCD_SEG6_PIN    GpioPin14
#define EVB_LCD_SEG7_PORT   GpioPortB
#define EVB_LCD_SEG7_PIN    GpioPin13

#define EVB_LCD_VLCDH_PORT  GpioPortB
#define EVB_LCD_VLCDH_PIN   GpioPin3
#define EVB_LCD_VLCD3_PORT  GpioPortB
#define EVB_LCD_VLCD3_PIN   GpioPin4
#define EVB_LCD_VLCD2_PORT  GpioPortB
#define EVB_LCD_VLCD2_PIN   GpioPin5
#define EVB_LCD_VLCD1_PORT  GpioPortB
#define EVB_LCD_VLCD1_PIN   GpioPin6

///< CAN
#define EVB_CAN_RX_PORT     GpioPortD
#define EVB_CAN_RX_PIN      GpioPin0
#define EVB_CAN_TX_PORT     GpioPortD
#define EVB_CAN_TX_PIN      GpioPin1
#define EVB_CAN_STB_PORT    GpioPortD
#define EVB_CAN_STB_PIN     GpioPin5

///< I2C EEPROM
#ifdef BOARD_EVBHC32LF07X_V11
#define EVB_I2C0_EEPROM_SCL_PORT    GpioPortB
#define EVB_I2C0_EEPROM_SCL_PIN     GpioPin8
#define EVB_I2C0_EEPROM_SDA_PORT    GpioPortB
#define EVB_I2C0_EEPROM_SDA_PIN     GpioPin9
#else
#define EVB_I2C0_EEPROM_SCL_PORT    GpioPortB
#define EVB_I2C0_EEPROM_SCL_PIN     GpioPin6
#define EVB_I2C0_EEPROM_SDA_PORT    GpioPortB
#define EVB_I2C0_EEPROM_SDA_PIN     GpioPin7
#endif

///< I2C CODEC
#define EVB_I2C1_CODEC_SCL_PORT    GpioPortB
#define EVB_I2C1_CODEC_SCL_PIN     GpioPin10
#define EVB_I2C1_CODEC_SDA_PORT    GpioPortB
#define EVB_I2C1_CODEC_SDA_PIN     GpioPin11

///< I2S CODEC
#define EVB_I2S0_CODEC_SCK_PORT     GpioPortD
#define EVB_I2S0_CODEC_SCK_PIN      GpioPin8
#define EVB_I2S0_CODEC_MCK_PORT     GpioPortD
#define EVB_I2S0_CODEC_MCK_PIN      GpioPin9
#define EVB_I2S0_CODEC_SD_PORT      GpioPortD
#define EVB_I2S0_CODEC_SD_PIN       GpioPin10
#define EVB_I2S0_CODEC_WS_PORT      GpioPortD
#define EVB_I2S0_CODEC_WS_PIN       GpioPin11

///< SPI FLASH

///< SPI0
#define EVB_SPI0_CS_PORT      GpioPortE
#define EVB_SPI0_CS_PIN       GpioPin12
#define EVB_SPI0_SCK_PORT     GpioPortE
#define EVB_SPI0_SCK_PIN      GpioPin13
#define EVB_SPI0_MISO_PORT    GpioPortE
#define EVB_SPI0_MISO_PIN     GpioPin14
#define EVB_SPI0_MOSI_PORT    GpioPortE
#define EVB_SPI0_MOSI_PIN     GpioPin15
///< SPI1
#define EVB_SPI1_CS_PORT      GpioPortB
#define EVB_SPI1_CS_PIN       GpioPin12
#define EVB_SPI1_SCK_PORT     GpioPortB
#define EVB_SPI1_SCK_PIN      GpioPin13
#define EVB_SPI1_MISO_PORT    GpioPortB
#define EVB_SPI1_MISO_PIN     GpioPin14
#define EVB_SPI1_MOSI_PORT    GpioPortB
#define EVB_SPI1_MOSI_PIN     GpioPin15

#ifdef BOARD_EVBHC32LF07X_V11
#define EVB_SPI1_FLASH_CS_PORT      GpioPortE
#define EVB_SPI1_FLASH_CS_PIN       GpioPin12
#define EVB_SPI1_FLASH_SCK_PORT     GpioPortE
#define EVB_SPI1_FLASH_SCK_PIN      GpioPin13
#define EVB_SPI1_FLASH_MISO_PORT    GpioPortE
#define EVB_SPI1_FLASH_MISO_PIN     GpioPin14
#define EVB_SPI1_FLASH_MOSI_PORT    GpioPortE
#define EVB_SPI1_FLASH_MOSI_PIN     GpioPin15
#else
#define EVB_SPI1_FLASH_CS_PORT      GpioPortB
#define EVB_SPI1_FLASH_CS_PIN       GpioPin12
#define EVB_SPI1_FLASH_SCK_PORT     GpioPortB
#define EVB_SPI1_FLASH_SCK_PIN      GpioPin13
#define EVB_SPI1_FLASH_MISO_PORT    GpioPortB
#define EVB_SPI1_FLASH_MISO_PIN     GpioPin14
#define EVB_SPI1_FLASH_MOSI_PORT    GpioPortB
#define EVB_SPI1_FLASH_MOSI_PIN     GpioPin15
#endif

#endif
