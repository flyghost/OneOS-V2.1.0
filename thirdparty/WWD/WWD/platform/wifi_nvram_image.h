/*
 * Broadcom Proprietary and Confidential. Copyright 2016 Broadcom
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/** @file
 *  NVRAM variables which define BCM43362 Parameters for the
 *  USI module used on the BCM943362WCD6_3 board
 *
 */

#ifndef INCLUDED_NVRAM_IMAGE_H_
#define INCLUDED_NVRAM_IMAGE_H_

#include <string.h>
#include <stdint.h>
//#include "../generated_mac_address.txt"
#define NVRAM_GENERATED_MAC_ADDRESS        "macaddr=02:0A:F7:7b:a4:ce"
#define DCT_GENERATED_MAC_ADDRESS          "\x02\x0A\xF7\xce\x7b\xa4"
#define DCT_GENERATED_ETHERNET_MAC_ADDRESS "\x02\x0A\xF7\xce\x7b\xa5"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Character array of NVRAM image
 */

static const char wifi_nvram_image[] =
"manfid=0x2d0"                                                  "\x00"
"prodid=0x492"                                                  "\x00"
"vendid=0x14e4"                                                 "\x00"
"devid=0x4343"                                                  "\x00"
"boardtype=0x0598"                                              "\x00"
"boardrev=0x1307"                                               "\x00"
"boardnum=777"                                                  "\x00"
"xtalfreq=26000"                                                "\x00"
"boardflags=0xa00"                                              "\x00"
"sromrev=3"                                                     "\x00"
"wl0id=0x431b"                                                  "\x00"
"macaddr=02:0A:F7:fe:86:1c"                                     "\x00"
"aa2g=1"                                                        "\x00"
"ag0=2"                                                         "\x00"
"maxp2ga0=74"                                                   "\x00"
"cck2gpo=0x2222"                                                "\x00"
"ofdm2gpo=0x66666666"                                           "\x00"
"mcs2gpo0=0x7777"                                               "\x00"
"mcs2gpo1=0x7777"                                               "\x00"
"pa0maxpwr=56"                                                  "\x00"
"pa0b0=5447"                                                    "\x00"
"pa0b1=-607"                                                    "\x00"
"pa0b2=-160"                                                    "\x00"
"pa0itssit=62"                                                  "\x00"
"pa1itssit=62"                                                  "\x00"
"temp_based_dutycy_en=1"                                        "\x00"
"tx_duty_cycle_ofdm=100"                                        "\x00"
"tx_duty_cycle_cck=100"                                         "\x00"
"tx_ofdm_temp_0=115"                                            "\x00"
"tx_cck_temp_0=115"                                             "\x00"
"tx_ofdm_dutycy_0=40"                                           "\x00"
"tx_cck_dutycy_0=40"                                            "\x00"
"tx_ofdm_temp_1=255"                                            "\x00"
"tx_cck_temp_1=255"                                             "\x00"
"tx_ofdm_dutycy_1=40"                                           "\x00"
"tx_cck_dutycy_1=40"                                            "\x00"
"tx_tone_power_index=40"                                        "\x00"
"tx_tone_power_index.fab.3=48"                                  "\x00"
"cckPwrOffset=5"                                                "\x00"
"ccode=0"                                                       "\x00"
"rssismf2g=0xf"                                                 "\x00"
"rssismc2g=0x8"                                                 "\x00"
"rssisav2g=0x1"                                                 "\x00"
"triso2g=0"                                                     "\x00"
"noise_cal_enable_2g=0"                                         "\x00"
"noise_cal_po_2g=0"                                             "\x00"
"noise_cal_po_2g.fab.3=-2"                                      "\x00"
"swctrlmap_2g=0x04040404,0x02020202,0x02020202,0x010101,0x1ff"  "\x00"
"temp_add=29767"                                                "\x00"
"temp_mult=425"                                                 "\x00"
"edonthd=-30"                                                   "\x00"
"edoffthd=-36"                                                  "\x00"
"temp_q=10"                                                     "\x00"
"initxidx2g=45"                                                 "\x00"
"tssitime=1"                                                    "\x00"
"rfreg033=0x19"                                                 "\x00"
"rfreg033_cck=0x1f"                                             "\x00"
"cckPwrIdxCorr=-8"                                              "\x00"
"spuravoid_enable2g=1"                                          "\x00"
"sd_gpout=1"                                                    "\x00"
"sd_oobonly=1"                                                  "\x00"
"sd_level_trigger=1"                                            "\x00"
"\x00\x00";


#else /* ifndef INCLUDED_NVRAM_IMAGE_H_ */

#error Wi-Fi NVRAM image included twice

#endif /* ifndef INCLUDED_NVRAM_IMAGE_H_ */

#ifdef __cplusplus
} /* extern "C" */
#endif
