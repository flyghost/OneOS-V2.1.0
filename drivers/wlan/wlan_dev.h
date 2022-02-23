/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with 
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * @file        wlan_dev.h
 *
 * @brief       This file implements wwd interface.
 *
 * @revision
 * Date         Author          Notes
 * 2021-08-12   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#ifndef __WLAN_DEV_H__
#define __WLAN_DEV_H__

#include <os_types.h>
#include <device.h>

#include "wwd_constants.h"

#define OS_WLAN_FLAG_STA_ONLY (0x1 << 0)
#define OS_WLAN_FLAG_AP_ONLY  (0x1 << 1)

#define OS_WEP_ENABLED            0x0001
#define OS_TKIP_ENABLED           0x0002
#define OS_AES_ENABLED            0x0004
#define OS_SHARED_ENABLED     0x00008000
#define OS_WPA_SECURITY       0x00200000
#define OS_WPA2_SECURITY      0x00400000
#define OS_ENTERPRISE_ENABLED 0x02000000
#define OS_WPS_ENABLED        0x10000000
#define OS_IBSS_ENABLED       0x20000000


#define OS_MK_CNTRY( a, b, rev )  (((unsigned char)(a)) + (((unsigned char)(b))<<8) + (((unsigned short)(rev))<<16))

#ifndef OS_WLAN_SSID_MAX_LENGTH
#define OS_WLAN_SSID_MAX_LENGTH (32) /* SSID MAX LEN */
#endif

#ifndef OS_WLAN_BSSID_MAX_LENGTH
#define OS_WLAN_BSSID_MAX_LENGTH (6) /* BSSID MAX LEN (default is 6) */
#endif

#ifndef OS_WLAN_PASSWORD_MAX_LENGTH
#define OS_WLAN_PASSWORD_MAX_LENGTH (32) /* PASSWORD MAX LEN*/
#endif

#ifndef OS_WLAN_DEV_EVENT_NUM
#define OS_WLAN_DEV_EVENT_NUM (2) /* EVENT GROUP MAX NUM */
#endif

struct os_wlan_device;

typedef enum
{
    OS_WLAN_SECURITY_OPEN           = 0,                                                                   /**< Open security                                         */
    OS_WLAN_SECURITY_WEP_PSK        = OS_WEP_ENABLED,                                                         /**< WEP PSK Security with open authentication             */
    OS_WLAN_SECURITY_WEP_SHARED     = ( OS_WEP_ENABLED   | OS_SHARED_ENABLED ),                                  /**< WEP PSK Security with shared authentication           */
    OS_WLAN_SECURITY_WPA_TKIP_PSK   = ( OS_WPA_SECURITY  | OS_TKIP_ENABLED ),                                    /**< WPA PSK Security with TKIP                            */
    OS_WLAN_SECURITY_WPA_AES_PSK    = ( OS_WPA_SECURITY  | OS_AES_ENABLED ),                                     /**< WPA PSK Security with AES                             */
    OS_WLAN_SECURITY_WPA_MIXED_PSK  = ( OS_WPA_SECURITY  | OS_AES_ENABLED | TKIP_ENABLED ),                      /**< WPA PSK Security with AES & TKIP                      */
    OS_WLAN_SECURITY_WPA2_AES_PSK   = ( OS_WPA2_SECURITY | OS_AES_ENABLED ),                                     /**< WPA2 PSK Security with AES                            */
    OS_WLAN_SECURITY_WPA2_TKIP_PSK  = ( OS_WPA2_SECURITY | OS_TKIP_ENABLED ),                                    /**< WPA2 PSK Security with TKIP                           */
    OS_WLAN_SECURITY_WPA2_MIXED_PSK = ( OS_WPA2_SECURITY | OS_AES_ENABLED | OS_TKIP_ENABLED ),                      /**< WPA2 PSK Security with AES & TKIP                     */

    OS_WLAN_SECURITY_WPA_TKIP_ENT   = ( OS_ENTERPRISE_ENABLED | OS_WPA_SECURITY  | OS_TKIP_ENABLED ),               /**< WPA Enterprise Security with TKIP                     */
    OS_WLAN_SECURITY_WPA_AES_ENT    = ( OS_ENTERPRISE_ENABLED | OS_WPA_SECURITY  | OS_AES_ENABLED ),                /**< WPA Enterprise Security with AES                      */
    OS_WLAN_SECURITY_WPA_MIXED_ENT  = ( OS_ENTERPRISE_ENABLED | OS_WPA_SECURITY  | OS_AES_ENABLED | OS_TKIP_ENABLED ), /**< WPA Enterprise Security with AES & TKIP               */
    OS_WLAN_SECURITY_WPA2_TKIP_ENT  = ( OS_ENTERPRISE_ENABLED | OS_WPA2_SECURITY | OS_TKIP_ENABLED ),               /**< WPA2 Enterprise Security with TKIP                    */
    OS_WLAN_SECURITY_WPA2_AES_ENT   = ( OS_ENTERPRISE_ENABLED | OS_WPA2_SECURITY | OS_AES_ENABLED ),                /**< WPA2 Enterprise Security with AES                     */
    OS_WLAN_SECURITY_WPA2_MIXED_ENT = ( OS_ENTERPRISE_ENABLED | OS_WPA2_SECURITY | OS_AES_ENABLED | OS_TKIP_ENABLED ), /**< WPA2 Enterprise Security with AES & TKIP              */

    OS_WLAN_SECURITY_IBSS_OPEN      = ( OS_IBSS_ENABLED ),                                  /**< Open security on IBSS ad-hoc network         */
    OS_WLAN_SECURITY_WPS_OPEN       = ( OS_WPS_ENABLED ),                                                     /**< WPS with open security                                */
    OS_WLAN_SECURITY_WPS_SECURE     = ( OS_WPS_ENABLED | OS_AES_ENABLED),                                        /**< WPS with AES security                                 */

    OS_WLAN_SECURITY_UNKNOWN        = -1,                                                                  /**< May be returned by scan function if security is unknown. Do not pass this to the join function! */

    OS_WLAN_SECURITY_FORCE_32_BIT   = 0x7fffffff                                                           /**< Exists only to force wiced_security_t type to 32 bits */
} os_wlan_security_t;

typedef enum
{
    OS_WLAN_COUNTRY_AFGHANISTAN                                     = OS_MK_CNTRY( 'A', 'F', 0 ),             /* AF Afghanistan */
    OS_WLAN_COUNTRY_ALBANIA                                         = OS_MK_CNTRY( 'A', 'L', 0 ),             /* AL Albania */
    OS_WLAN_COUNTRY_ALGERIA                                         = OS_MK_CNTRY( 'D', 'Z', 0 ),             /* DZ Algeria */
    OS_WLAN_COUNTRY_AMERICAN_SAMOA                                  = OS_MK_CNTRY( 'A', 'S', 0 ),             /* AS American_Samoa */
    OS_WLAN_COUNTRY_ANGOLA                                          = OS_MK_CNTRY( 'A', 'O', 0 ),             /* AO Angola */
    OS_WLAN_COUNTRY_ANGUILLA                                        = OS_MK_CNTRY( 'A', 'I', 0 ),             /* AI Anguilla */
    OS_WLAN_COUNTRY_ANTIGUA_AND_BARBUDA                             = OS_MK_CNTRY( 'A', 'G', 0 ),             /* AG Antigua_and_Barbuda */
    OS_WLAN_COUNTRY_ARGENTINA                                       = OS_MK_CNTRY( 'A', 'R', 0 ),             /* AR Argentina */
    OS_WLAN_COUNTRY_ARMENIA                                         = OS_MK_CNTRY( 'A', 'M', 0 ),             /* AM Armenia */
    OS_WLAN_COUNTRY_ARUBA                                           = OS_MK_CNTRY( 'A', 'W', 0 ),             /* AW Aruba */
    OS_WLAN_COUNTRY_AUSTRALIA                                       = OS_MK_CNTRY( 'A', 'U', 0 ),             /* AU Australia */
    OS_WLAN_COUNTRY_AUSTRIA                                         = OS_MK_CNTRY( 'A', 'T', 0 ),             /* AT Austria */
    OS_WLAN_COUNTRY_AZERBAIJAN                                      = OS_MK_CNTRY( 'A', 'Z', 0 ),             /* AZ Azerbaijan */
    OS_WLAN_COUNTRY_BAHAMAS                                         = OS_MK_CNTRY( 'B', 'S', 0 ),             /* BS Bahamas */
    OS_WLAN_COUNTRY_BAHRAIN                                         = OS_MK_CNTRY( 'B', 'H', 0 ),             /* BH Bahrain */
    OS_WLAN_COUNTRY_BAKER_ISLAND                                    = OS_MK_CNTRY( '0', 'B', 0 ),             /* 0B Baker_Island */
    OS_WLAN_COUNTRY_BANGLADESH                                      = OS_MK_CNTRY( 'B', 'D', 0 ),             /* BD Bangladesh */
    OS_WLAN_COUNTRY_BARBADOS                                        = OS_MK_CNTRY( 'B', 'B', 0 ),             /* BB Barbados */
    OS_WLAN_COUNTRY_BELARUS                                         = OS_MK_CNTRY( 'B', 'Y', 0 ),             /* BY Belarus */
    OS_WLAN_COUNTRY_BELGIUM                                         = OS_MK_CNTRY( 'B', 'E', 0 ),             /* BE Belgium */
    OS_WLAN_COUNTRY_BELIZE                                          = OS_MK_CNTRY( 'B', 'Z', 0 ),             /* BZ Belize */
    OS_WLAN_COUNTRY_BENIN                                           = OS_MK_CNTRY( 'B', 'J', 0 ),             /* BJ Benin */
    OS_WLAN_COUNTRY_BERMUDA                                         = OS_MK_CNTRY( 'B', 'M', 0 ),             /* BM Bermuda */
    OS_WLAN_COUNTRY_BHUTAN                                          = OS_MK_CNTRY( 'B', 'T', 0 ),             /* BT Bhutan */
    OS_WLAN_COUNTRY_BOLIVIA                                         = OS_MK_CNTRY( 'B', 'O', 0 ),             /* BO Bolivia */
    OS_WLAN_COUNTRY_BOSNIA_AND_HERZEGOVINA                          = OS_MK_CNTRY( 'B', 'A', 0 ),             /* BA Bosnia_and_Herzegovina */
    OS_WLAN_COUNTRY_BOTSWANA                                        = OS_MK_CNTRY( 'B', 'W', 0 ),             /* BW Botswana */
    OS_WLAN_COUNTRY_BRAZIL                                          = OS_MK_CNTRY( 'B', 'R', 0 ),             /* BR Brazil */
    OS_WLAN_COUNTRY_BRITISH_INDIAN_OCEAN_TERRITORY                  = OS_MK_CNTRY( 'I', 'O', 0 ),             /* IO British_Indian_Ocean_Territory */
    OS_WLAN_COUNTRY_BRUNEI_DARUSSALAM                               = OS_MK_CNTRY( 'B', 'N', 0 ),             /* BN Brunei_Darussalam */
    OS_WLAN_COUNTRY_BULGARIA                                        = OS_MK_CNTRY( 'B', 'G', 0 ),             /* BG Bulgaria */
    OS_WLAN_COUNTRY_BURKINA_FASO                                    = OS_MK_CNTRY( 'B', 'F', 0 ),             /* BF Burkina_Faso */
    OS_WLAN_COUNTRY_BURUNDI                                         = OS_MK_CNTRY( 'B', 'I', 0 ),             /* BI Burundi */
    OS_WLAN_COUNTRY_CAMBODIA                                        = OS_MK_CNTRY( 'K', 'H', 0 ),             /* KH Cambodia */
    OS_WLAN_COUNTRY_CAMEROON                                        = OS_MK_CNTRY( 'C', 'M', 0 ),             /* CM Cameroon */
    OS_WLAN_COUNTRY_CANADA                                          = OS_MK_CNTRY( 'C', 'A', 0 ),             /* CA Canada */
    OS_WLAN_COUNTRY_CAPE_VERDE                                      = OS_MK_CNTRY( 'C', 'V', 0 ),             /* CV Cape_Verde */
    OS_WLAN_COUNTRY_CAYMAN_ISLANDS                                  = OS_MK_CNTRY( 'K', 'Y', 0 ),             /* KY Cayman_Islands */
    OS_WLAN_COUNTRY_CENTRAL_AFRICAN_REPUBLIC                        = OS_MK_CNTRY( 'C', 'F', 0 ),             /* CF Central_African_Republic */
    OS_WLAN_COUNTRY_CHAD                                            = OS_MK_CNTRY( 'T', 'D', 0 ),             /* TD Chad */
    OS_WLAN_COUNTRY_CHILE                                           = OS_MK_CNTRY( 'C', 'L', 0 ),             /* CL Chile */
    OS_WLAN_COUNTRY_CHINA                                           = OS_MK_CNTRY( 'C', 'N', 0 ),             /* CN China */
    OS_WLAN_COUNTRY_CHRISTMAS_ISLAND                                = OS_MK_CNTRY( 'C', 'X', 0 ),             /* CX Christmas_Island */
    OS_WLAN_COUNTRY_COLOMBIA                                        = OS_MK_CNTRY( 'C', 'O', 0 ),             /* CO Colombia */
    OS_WLAN_COUNTRY_COMOROS                                         = OS_MK_CNTRY( 'K', 'M', 0 ),             /* KM Comoros */
    OS_WLAN_COUNTRY_CONGO                                           = OS_MK_CNTRY( 'C', 'G', 0 ),             /* CG Congo */
    OS_WLAN_COUNTRY_CONGO_THE_DEMOCRATIC_REPUBLIC_OF_THE            = OS_MK_CNTRY( 'C', 'D', 0 ),             /* CD Congo,_The_Democratic_Republic_Of_The */
    OS_WLAN_COUNTRY_COSTA_RICA                                      = OS_MK_CNTRY( 'C', 'R', 0 ),             /* CR Costa_Rica */
    OS_WLAN_COUNTRY_COTE_DIVOIRE                                    = OS_MK_CNTRY( 'C', 'I', 0 ),             /* CI Cote_D'ivoire */
    OS_WLAN_COUNTRY_CROATIA                                         = OS_MK_CNTRY( 'H', 'R', 0 ),             /* HR Croatia */
    OS_WLAN_COUNTRY_CUBA                                            = OS_MK_CNTRY( 'C', 'U', 0 ),             /* CU Cuba */
    OS_WLAN_COUNTRY_CYPRUS                                          = OS_MK_CNTRY( 'C', 'Y', 0 ),             /* CY Cyprus */
    OS_WLAN_COUNTRY_CZECH_REPUBLIC                                  = OS_MK_CNTRY( 'C', 'Z', 0 ),             /* CZ Czech_Republic */
    OS_WLAN_COUNTRY_DENMARK                                         = OS_MK_CNTRY( 'D', 'K', 0 ),             /* DK Denmark */
    OS_WLAN_COUNTRY_DJIBOUTI                                        = OS_MK_CNTRY( 'D', 'J', 0 ),             /* DJ Djibouti */
    OS_WLAN_COUNTRY_DOMINICA                                        = OS_MK_CNTRY( 'D', 'M', 0 ),             /* DM Dominica */
    OS_WLAN_COUNTRY_DOMINICAN_REPUBLIC                              = OS_MK_CNTRY( 'D', 'O', 0 ),             /* DO Dominican_Republic */
    OS_WLAN_COUNTRY_DOWN_UNDER                                      = OS_MK_CNTRY( 'A', 'U', 0 ),             /* AU G'Day mate! */
    OS_WLAN_COUNTRY_ECUADOR                                         = OS_MK_CNTRY( 'E', 'C', 0 ),             /* EC Ecuador */
    OS_WLAN_COUNTRY_EGYPT                                           = OS_MK_CNTRY( 'E', 'G', 0 ),             /* EG Egypt */
    OS_WLAN_COUNTRY_EL_SALVADOR                                     = OS_MK_CNTRY( 'S', 'V', 0 ),             /* SV El_Salvador */
    OS_WLAN_COUNTRY_EQUATORIAL_GUINEA                               = OS_MK_CNTRY( 'G', 'Q', 0 ),             /* GQ Equatorial_Guinea */
    OS_WLAN_COUNTRY_ERITREA                                         = OS_MK_CNTRY( 'E', 'R', 0 ),             /* ER Eritrea */
    OS_WLAN_COUNTRY_ESTONIA                                         = OS_MK_CNTRY( 'E', 'E', 0 ),             /* EE Estonia */
    OS_WLAN_COUNTRY_ETHIOPIA                                        = OS_MK_CNTRY( 'E', 'T', 0 ),             /* ET Ethiopia */
    OS_WLAN_COUNTRY_FALKLAND_ISLANDS_MALVINAS                       = OS_MK_CNTRY( 'F', 'K', 0 ),             /* FK Falkland_Islands_(Malvinas) */
    OS_WLAN_COUNTRY_FAROE_ISLANDS                                   = OS_MK_CNTRY( 'F', 'O', 0 ),             /* FO Faroe_Islands */
    OS_WLAN_COUNTRY_FIJI                                            = OS_MK_CNTRY( 'F', 'J', 0 ),             /* FJ Fiji */
    OS_WLAN_COUNTRY_FINLAND                                         = OS_MK_CNTRY( 'F', 'I', 0 ),             /* FI Finland */
    OS_WLAN_COUNTRY_FRANCE                                          = OS_MK_CNTRY( 'F', 'R', 0 ),             /* FR France */
    OS_WLAN_COUNTRY_FRENCH_GUINA                                    = OS_MK_CNTRY( 'G', 'F', 0 ),             /* GF French_Guina */
    OS_WLAN_COUNTRY_FRENCH_POLYNESIA                                = OS_MK_CNTRY( 'P', 'F', 0 ),             /* PF French_Polynesia */
    OS_WLAN_COUNTRY_FRENCH_SOUTHERN_TERRITORIES                     = OS_MK_CNTRY( 'T', 'F', 0 ),             /* TF French_Southern_Territories */
    OS_WLAN_COUNTRY_GABON                                           = OS_MK_CNTRY( 'G', 'A', 0 ),             /* GA Gabon */
    OS_WLAN_COUNTRY_GAMBIA                                          = OS_MK_CNTRY( 'G', 'M', 0 ),             /* GM Gambia */
    OS_WLAN_COUNTRY_GEORGIA                                         = OS_MK_CNTRY( 'G', 'E', 0 ),             /* GE Georgia */
    OS_WLAN_COUNTRY_GERMANY                                         = OS_MK_CNTRY( 'D', 'E', 0 ),             /* DE Germany */
    OS_WLAN_COUNTRY_GHANA                                           = OS_MK_CNTRY( 'G', 'H', 0 ),             /* GH Ghana */
    OS_WLAN_COUNTRY_GIBRALTAR                                       = OS_MK_CNTRY( 'G', 'I', 0 ),             /* GI Gibraltar */
    OS_WLAN_COUNTRY_GREECE                                          = OS_MK_CNTRY( 'G', 'R', 0 ),             /* GR Greece */
    OS_WLAN_COUNTRY_GRENADA                                         = OS_MK_CNTRY( 'G', 'D', 0 ),             /* GD Grenada */
    OS_WLAN_COUNTRY_GUADELOUPE                                      = OS_MK_CNTRY( 'G', 'P', 0 ),             /* GP Guadeloupe */
    OS_WLAN_COUNTRY_GUAM                                            = OS_MK_CNTRY( 'G', 'U', 0 ),             /* GU Guam */
    OS_WLAN_COUNTRY_GUATEMALA                                       = OS_MK_CNTRY( 'G', 'T', 0 ),             /* GT Guatemala */
    OS_WLAN_COUNTRY_GUERNSEY                                        = OS_MK_CNTRY( 'G', 'G', 0 ),             /* GG Guernsey */
    OS_WLAN_COUNTRY_GUINEA                                          = OS_MK_CNTRY( 'G', 'N', 0 ),             /* GN Guinea */
    OS_WLAN_COUNTRY_GUINEA_BISSAU                                   = OS_MK_CNTRY( 'G', 'W', 0 ),             /* GW Guinea-bissau */
    OS_WLAN_COUNTRY_GUYANA                                          = OS_MK_CNTRY( 'G', 'Y', 0 ),             /* GY Guyana */
    OS_WLAN_COUNTRY_HAITI                                           = OS_MK_CNTRY( 'H', 'T', 0 ),             /* HT Haiti */
    OS_WLAN_COUNTRY_HOLY_SEE_VATICAN_CITY_STATE                     = OS_MK_CNTRY( 'V', 'A', 0 ),             /* VA Holy_See_(Vatican_City_State) */
    OS_WLAN_COUNTRY_HONDURAS                                        = OS_MK_CNTRY( 'H', 'N', 0 ),             /* HN Honduras */
    OS_WLAN_COUNTRY_HONG_KONG                                       = OS_MK_CNTRY( 'H', 'K', 0 ),             /* HK Hong_Kong */
    OS_WLAN_COUNTRY_HUNGARY                                         = OS_MK_CNTRY( 'H', 'U', 0 ),             /* HU Hungary */
    OS_WLAN_COUNTRY_ICELAND                                         = OS_MK_CNTRY( 'I', 'S', 0 ),             /* IS Iceland */
    OS_WLAN_COUNTRY_INDIA                                           = OS_MK_CNTRY( 'I', 'N', 0 ),             /* IN India */
    OS_WLAN_COUNTRY_INDONESIA                                       = OS_MK_CNTRY( 'I', 'D', 0 ),             /* ID Indonesia */
    OS_WLAN_COUNTRY_IRAN_ISLAMIC_REPUBLIC_OF                        = OS_MK_CNTRY( 'I', 'R', 0 ),             /* IR Iran,_Islamic_Republic_Of */
    OS_WLAN_COUNTRY_IRAQ                                            = OS_MK_CNTRY( 'I', 'Q', 0 ),             /* IQ Iraq */
    OS_WLAN_COUNTRY_IRELAND                                         = OS_MK_CNTRY( 'I', 'E', 0 ),             /* IE Ireland */
    OS_WLAN_COUNTRY_ISRAEL                                          = OS_MK_CNTRY( 'I', 'L', 0 ),             /* IL Israel */
    OS_WLAN_COUNTRY_ITALY                                           = OS_MK_CNTRY( 'I', 'T', 0 ),             /* IT Italy */
    OS_WLAN_COUNTRY_JAMAICA                                         = OS_MK_CNTRY( 'J', 'M', 0 ),             /* JM Jamaica */
    OS_WLAN_COUNTRY_JAPAN                                           = OS_MK_CNTRY( 'J', 'P', 0 ),             /* JP Japan */
    OS_WLAN_COUNTRY_JERSEY                                          = OS_MK_CNTRY( 'J', 'E', 0 ),             /* JE Jersey */
    OS_WLAN_COUNTRY_JORDAN                                          = OS_MK_CNTRY( 'J', 'O', 0 ),             /* JO Jordan */
    OS_WLAN_COUNTRY_KAZAKHSTAN                                      = OS_MK_CNTRY( 'K', 'Z', 0 ),             /* KZ Kazakhstan */
    OS_WLAN_COUNTRY_KENYA                                           = OS_MK_CNTRY( 'K', 'E', 0 ),             /* KE Kenya */
    OS_WLAN_COUNTRY_KIRIBATI                                        = OS_MK_CNTRY( 'K', 'I', 0 ),             /* KI Kiribati */
    OS_WLAN_COUNTRY_KOREA_REPUBLIC_OF                               = OS_MK_CNTRY( 'K', 'R', 1 ),             /* KR Korea,_Republic_Of */
    OS_WLAN_COUNTRY_KOSOVO                                          = OS_MK_CNTRY( '0', 'A', 0 ),             /* 0A Kosovo */
    OS_WLAN_COUNTRY_KUWAIT                                          = OS_MK_CNTRY( 'K', 'W', 0 ),             /* KW Kuwait */
    OS_WLAN_COUNTRY_KYRGYZSTAN                                      = OS_MK_CNTRY( 'K', 'G', 0 ),             /* KG Kyrgyzstan */
    OS_WLAN_COUNTRY_LAO_PEOPLES_DEMOCRATIC_REPUBIC                  = OS_MK_CNTRY( 'L', 'A', 0 ),             /* LA Lao_People's_Democratic_Repubic */
    OS_WLAN_COUNTRY_LATVIA                                          = OS_MK_CNTRY( 'L', 'V', 0 ),             /* LV Latvia */
    OS_WLAN_COUNTRY_LEBANON                                         = OS_MK_CNTRY( 'L', 'B', 0 ),             /* LB Lebanon */
    OS_WLAN_COUNTRY_LESOTHO                                         = OS_MK_CNTRY( 'L', 'S', 0 ),             /* LS Lesotho */
    OS_WLAN_COUNTRY_LIBERIA                                         = OS_MK_CNTRY( 'L', 'R', 0 ),             /* LR Liberia */
    OS_WLAN_COUNTRY_LIBYAN_ARAB_JAMAHIRIYA                          = OS_MK_CNTRY( 'L', 'Y', 0 ),             /* LY Libyan_Arab_Jamahiriya */
    OS_WLAN_COUNTRY_LIECHTENSTEIN                                   = OS_MK_CNTRY( 'L', 'I', 0 ),             /* LI Liechtenstein */
    OS_WLAN_COUNTRY_LITHUANIA                                       = OS_MK_CNTRY( 'L', 'T', 0 ),             /* LT Lithuania */
    OS_WLAN_COUNTRY_LUXEMBOURG                                      = OS_MK_CNTRY( 'L', 'U', 0 ),             /* LU Luxembourg */
    OS_WLAN_COUNTRY_MACAO                                           = OS_MK_CNTRY( 'M', 'O', 0 ),             /* MO Macao */
    OS_WLAN_COUNTRY_MACEDONIA_FORMER_YUGOSLAV_REPUBLIC_OF           = OS_MK_CNTRY( 'M', 'K', 0 ),             /* MK Macedonia,_Former_Yugoslav_Republic_Of */
    OS_WLAN_COUNTRY_MADAGASCAR                                      = OS_MK_CNTRY( 'M', 'G', 0 ),             /* MG Madagascar */
    OS_WLAN_COUNTRY_MALAWI                                          = OS_MK_CNTRY( 'M', 'W', 0 ),             /* MW Malawi */
    OS_WLAN_COUNTRY_MALAYSIA                                        = OS_MK_CNTRY( 'M', 'Y', 0 ),             /* MY Malaysia */
    OS_WLAN_COUNTRY_MALDIVES                                        = OS_MK_CNTRY( 'M', 'V', 0 ),             /* MV Maldives */
    OS_WLAN_COUNTRY_MALI                                            = OS_MK_CNTRY( 'M', 'L', 0 ),             /* ML Mali */
    OS_WLAN_COUNTRY_MALTA                                           = OS_MK_CNTRY( 'M', 'T', 0 ),             /* MT Malta */
    OS_WLAN_COUNTRY_MAN_ISLE_OF                                     = OS_MK_CNTRY( 'I', 'M', 0 ),             /* IM Man,_Isle_Of */
    OS_WLAN_COUNTRY_MARTINIQUE                                      = OS_MK_CNTRY( 'M', 'Q', 0 ),             /* MQ Martinique */
    OS_WLAN_COUNTRY_MAURITANIA                                      = OS_MK_CNTRY( 'M', 'R', 0 ),             /* MR Mauritania */
    OS_WLAN_COUNTRY_MAURITIUS                                       = OS_MK_CNTRY( 'M', 'U', 0 ),             /* MU Mauritius */
    OS_WLAN_COUNTRY_MAYOTTE                                         = OS_MK_CNTRY( 'Y', 'T', 0 ),             /* YT Mayotte */
    OS_WLAN_COUNTRY_MEXICO                                          = OS_MK_CNTRY( 'M', 'X', 0 ),             /* MX Mexico */
    OS_WLAN_COUNTRY_MICRONESIA_FEDERATED_STATES_OF                  = OS_MK_CNTRY( 'F', 'M', 0 ),             /* FM Micronesia,_Federated_States_Of */
    OS_WLAN_COUNTRY_MOLDOVA_REPUBLIC_OF                             = OS_MK_CNTRY( 'M', 'D', 0 ),             /* MD Moldova,_Republic_Of */
    OS_WLAN_COUNTRY_MONACO                                          = OS_MK_CNTRY( 'M', 'C', 0 ),             /* MC Monaco */
    OS_WLAN_COUNTRY_MONGOLIA                                        = OS_MK_CNTRY( 'M', 'N', 0 ),             /* MN Mongolia */
    OS_WLAN_COUNTRY_MONTENEGRO                                      = OS_MK_CNTRY( 'M', 'E', 0 ),             /* ME Montenegro */
    OS_WLAN_COUNTRY_MONTSERRAT                                      = OS_MK_CNTRY( 'M', 'S', 0 ),             /* MS Montserrat */
    OS_WLAN_COUNTRY_MOROCCO                                         = OS_MK_CNTRY( 'M', 'A', 0 ),             /* MA Morocco */
    OS_WLAN_COUNTRY_MOZAMBIQUE                                      = OS_MK_CNTRY( 'M', 'Z', 0 ),             /* MZ Mozambique */
    OS_WLAN_COUNTRY_MYANMAR                                         = OS_MK_CNTRY( 'M', 'M', 0 ),             /* MM Myanmar */
    OS_WLAN_COUNTRY_NAMIBIA                                         = OS_MK_CNTRY( 'N', 'A', 0 ),             /* NA Namibia */
    OS_WLAN_COUNTRY_NAURU                                           = OS_MK_CNTRY( 'N', 'R', 0 ),             /* NR Nauru */
    OS_WLAN_COUNTRY_NEPAL                                           = OS_MK_CNTRY( 'N', 'P', 0 ),             /* NP Nepal */
    OS_WLAN_COUNTRY_NETHERLANDS                                     = OS_MK_CNTRY( 'N', 'L', 0 ),             /* NL Netherlands */
    OS_WLAN_COUNTRY_NETHERLANDS_ANTILLES                            = OS_MK_CNTRY( 'A', 'N', 0 ),             /* AN Netherlands_Antilles */
    OS_WLAN_COUNTRY_NEW_CALEDONIA                                   = OS_MK_CNTRY( 'N', 'C', 0 ),             /* NC New_Caledonia */
    OS_WLAN_COUNTRY_NEW_ZEALAND                                     = OS_MK_CNTRY( 'N', 'Z', 0 ),             /* NZ New_Zealand */
    OS_WLAN_COUNTRY_NICARAGUA                                       = OS_MK_CNTRY( 'N', 'I', 0 ),             /* NI Nicaragua */
    OS_WLAN_COUNTRY_NIGER                                           = OS_MK_CNTRY( 'N', 'E', 0 ),             /* NE Niger */
    OS_WLAN_COUNTRY_NIGERIA                                         = OS_MK_CNTRY( 'N', 'G', 0 ),             /* NG Nigeria */
    OS_WLAN_COUNTRY_NORFOLK_ISLAND                                  = OS_MK_CNTRY( 'N', 'F', 0 ),             /* NF Norfolk_Island */
    OS_WLAN_COUNTRY_NORTHERN_MARIANA_ISLANDS                        = OS_MK_CNTRY( 'M', 'P', 0 ),             /* MP Northern_Mariana_Islands */
    OS_WLAN_COUNTRY_NORWAY                                          = OS_MK_CNTRY( 'N', 'O', 0 ),             /* NO Norway */
    OS_WLAN_COUNTRY_OMAN                                            = OS_MK_CNTRY( 'O', 'M', 0 ),             /* OM Oman */
    OS_WLAN_COUNTRY_PAKISTAN                                        = OS_MK_CNTRY( 'P', 'K', 0 ),             /* PK Pakistan */
    OS_WLAN_COUNTRY_PALAU                                           = OS_MK_CNTRY( 'P', 'W', 0 ),             /* PW Palau */
    OS_WLAN_COUNTRY_PANAMA                                          = OS_MK_CNTRY( 'P', 'A', 0 ),             /* PA Panama */
    OS_WLAN_COUNTRY_PAPUA_NEW_GUINEA                                = OS_MK_CNTRY( 'P', 'G', 0 ),             /* PG Papua_New_Guinea */
    OS_WLAN_COUNTRY_PARAGUAY                                        = OS_MK_CNTRY( 'P', 'Y', 0 ),             /* PY Paraguay */
    OS_WLAN_COUNTRY_PERU                                            = OS_MK_CNTRY( 'P', 'E', 0 ),             /* PE Peru */
    OS_WLAN_COUNTRY_PHILIPPINES                                     = OS_MK_CNTRY( 'P', 'H', 0 ),             /* PH Philippines */
    OS_WLAN_COUNTRY_POLAND                                          = OS_MK_CNTRY( 'P', 'L', 0 ),             /* PL Poland */
    OS_WLAN_COUNTRY_PORTUGAL                                        = OS_MK_CNTRY( 'P', 'T', 0 ),             /* PT Portugal */
    OS_WLAN_COUNTRY_PUETO_RICO                                      = OS_MK_CNTRY( 'P', 'R', 0 ),             /* PR Pueto_Rico */
    OS_WLAN_COUNTRY_QATAR                                           = OS_MK_CNTRY( 'Q', 'A', 0 ),             /* QA Qatar */
    OS_WLAN_COUNTRY_REUNION                                         = OS_MK_CNTRY( 'R', 'E', 0 ),             /* RE Reunion */
    OS_WLAN_COUNTRY_ROMANIA                                         = OS_MK_CNTRY( 'R', 'O', 0 ),             /* RO Romania */
    OS_WLAN_COUNTRY_RUSSIAN_FEDERATION                              = OS_MK_CNTRY( 'R', 'U', 0 ),             /* RU Russian_Federation */
    OS_WLAN_COUNTRY_RWANDA                                          = OS_MK_CNTRY( 'R', 'W', 0 ),             /* RW Rwanda */
    OS_WLAN_COUNTRY_SAINT_KITTS_AND_NEVIS                           = OS_MK_CNTRY( 'K', 'N', 0 ),             /* KN Saint_Kitts_and_Nevis */
    OS_WLAN_COUNTRY_SAINT_LUCIA                                     = OS_MK_CNTRY( 'L', 'C', 0 ),             /* LC Saint_Lucia */
    OS_WLAN_COUNTRY_SAINT_PIERRE_AND_MIQUELON                       = OS_MK_CNTRY( 'P', 'M', 0 ),             /* PM Saint_Pierre_and_Miquelon */
    OS_WLAN_COUNTRY_SAINT_VINCENT_AND_THE_GRENADINES                = OS_MK_CNTRY( 'V', 'C', 0 ),             /* VC Saint_Vincent_and_The_Grenadines */
    OS_WLAN_COUNTRY_SAMOA                                           = OS_MK_CNTRY( 'W', 'S', 0 ),             /* WS Samoa */
    OS_WLAN_COUNTRY_SANIT_MARTIN_SINT_MARTEEN                       = OS_MK_CNTRY( 'M', 'F', 0 ),             /* MF Sanit_Martin_/_Sint_Marteen */
    OS_WLAN_COUNTRY_SAO_TOME_AND_PRINCIPE                           = OS_MK_CNTRY( 'S', 'T', 0 ),             /* ST Sao_Tome_and_Principe */
    OS_WLAN_COUNTRY_SAUDI_ARABIA                                    = OS_MK_CNTRY( 'S', 'A', 0 ),             /* SA Saudi_Arabia */
    OS_WLAN_COUNTRY_SENEGAL                                         = OS_MK_CNTRY( 'S', 'N', 0 ),             /* SN Senegal */
    OS_WLAN_COUNTRY_SERBIA                                          = OS_MK_CNTRY( 'R', 'S', 0 ),             /* RS Serbia */
    OS_WLAN_COUNTRY_SEYCHELLES                                      = OS_MK_CNTRY( 'S', 'C', 0 ),             /* SC Seychelles */
    OS_WLAN_COUNTRY_SIERRA_LEONE                                    = OS_MK_CNTRY( 'S', 'L', 0 ),             /* SL Sierra_Leone */
    OS_WLAN_COUNTRY_SINGAPORE                                       = OS_MK_CNTRY( 'S', 'G', 0 ),             /* SG Singapore */
    OS_WLAN_COUNTRY_SLOVAKIA                                        = OS_MK_CNTRY( 'S', 'K', 0 ),             /* SK Slovakia */
    OS_WLAN_COUNTRY_SLOVENIA                                        = OS_MK_CNTRY( 'S', 'I', 0 ),             /* SI Slovenia */
    OS_WLAN_COUNTRY_SOLOMON_ISLANDS                                 = OS_MK_CNTRY( 'S', 'B', 0 ),             /* SB Solomon_Islands */
    OS_WLAN_COUNTRY_SOMALIA                                         = OS_MK_CNTRY( 'S', 'O', 0 ),             /* SO Somalia */
    OS_WLAN_COUNTRY_SOUTH_AFRICA                                    = OS_MK_CNTRY( 'Z', 'A', 0 ),             /* ZA South_Africa */
    OS_WLAN_COUNTRY_SPAIN                                           = OS_MK_CNTRY( 'E', 'S', 0 ),             /* ES Spain */
    OS_WLAN_COUNTRY_SRI_LANKA                                       = OS_MK_CNTRY( 'L', 'K', 0 ),             /* LK Sri_Lanka */
    OS_WLAN_COUNTRY_SURINAME                                        = OS_MK_CNTRY( 'S', 'R', 0 ),             /* SR Suriname */
    OS_WLAN_COUNTRY_SWAZILAND                                       = OS_MK_CNTRY( 'S', 'Z', 0 ),             /* SZ Swaziland */
    OS_WLAN_COUNTRY_SWEDEN                                          = OS_MK_CNTRY( 'S', 'E', 0 ),             /* SE Sweden */
    OS_WLAN_COUNTRY_SWITZERLAND                                     = OS_MK_CNTRY( 'C', 'H', 0 ),             /* CH Switzerland */
    OS_WLAN_COUNTRY_SYRIAN_ARAB_REPUBLIC                            = OS_MK_CNTRY( 'S', 'Y', 0 ),             /* SY Syrian_Arab_Republic */
    OS_WLAN_COUNTRY_TAIWAN_PROVINCE_OF_CHINA                        = OS_MK_CNTRY( 'T', 'W', 0 ),             /* TW Taiwan,_Province_Of_China */
    OS_WLAN_COUNTRY_TAJIKISTAN                                      = OS_MK_CNTRY( 'T', 'J', 0 ),             /* TJ Tajikistan */
    OS_WLAN_COUNTRY_TANZANIA_UNITED_REPUBLIC_OF                     = OS_MK_CNTRY( 'T', 'Z', 0 ),             /* TZ Tanzania,_United_Republic_Of */
    OS_WLAN_COUNTRY_THAILAND                                        = OS_MK_CNTRY( 'T', 'H', 0 ),             /* TH Thailand */
    OS_WLAN_COUNTRY_TOGO                                            = OS_MK_CNTRY( 'T', 'G', 0 ),             /* TG Togo */
    OS_WLAN_COUNTRY_TONGA                                           = OS_MK_CNTRY( 'T', 'O', 0 ),             /* TO Tonga */
    OS_WLAN_COUNTRY_TRINIDAD_AND_TOBAGO                             = OS_MK_CNTRY( 'T', 'T', 0 ),             /* TT Trinidad_and_Tobago */
    OS_WLAN_COUNTRY_TUNISIA                                         = OS_MK_CNTRY( 'T', 'N', 0 ),             /* TN Tunisia */
    OS_WLAN_COUNTRY_TURKEY                                          = OS_MK_CNTRY( 'T', 'R', 0 ),             /* TR Turkey */
    OS_WLAN_COUNTRY_TURKMENISTAN                                    = OS_MK_CNTRY( 'T', 'M', 0 ),             /* TM Turkmenistan */
    OS_WLAN_COUNTRY_TURKS_AND_CAICOS_ISLANDS                        = OS_MK_CNTRY( 'T', 'C', 0 ),             /* TC Turks_and_Caicos_Islands */
    OS_WLAN_COUNTRY_TUVALU                                          = OS_MK_CNTRY( 'T', 'V', 0 ),             /* TV Tuvalu */
    OS_WLAN_COUNTRY_UGANDA                                          = OS_MK_CNTRY( 'U', 'G', 0 ),             /* UG Uganda */
    OS_WLAN_COUNTRY_UKRAINE                                         = OS_MK_CNTRY( 'U', 'A', 0 ),             /* UA Ukraine */
    OS_WLAN_COUNTRY_UNITED_ARAB_EMIRATES                            = OS_MK_CNTRY( 'A', 'E', 0 ),             /* AE United_Arab_Emirates */
    OS_WLAN_COUNTRY_UNITED_KINGDOM                                  = OS_MK_CNTRY( 'G', 'B', 0 ),             /* GB United_Kingdom */
    OS_WLAN_COUNTRY_UNITED_STATES                                   = OS_MK_CNTRY( 'U', 'S', 0 ),             /* US United_States */
    OS_WLAN_COUNTRY_UNITED_STATES_REV4                              = OS_MK_CNTRY( 'U', 'S', 4 ),             /* US United_States Revision 4 */
    OS_WLAN_COUNTRY_UNITED_STATES_REV931                            = OS_MK_CNTRY( 'Q', '1', 931 ),           /* Q1 United_States Revision 931 */
    OS_WLAN_COUNTRY_UNITED_STATES_NO_DFS                            = OS_MK_CNTRY( 'Q', '2', 0 ),             /* Q2 United_States_(No_DFS) */
    OS_WLAN_COUNTRY_UNITED_STATES_MINOR_OUTLYING_ISLANDS            = OS_MK_CNTRY( 'U', 'M', 0 ),             /* UM United_States_Minor_Outlying_Islands */
    OS_WLAN_COUNTRY_URUGUAY                                         = OS_MK_CNTRY( 'U', 'Y', 0 ),             /* UY Uruguay */
    OS_WLAN_COUNTRY_UZBEKISTAN                                      = OS_MK_CNTRY( 'U', 'Z', 0 ),             /* UZ Uzbekistan */
    OS_WLAN_COUNTRY_VANUATU                                         = OS_MK_CNTRY( 'V', 'U', 0 ),             /* VU Vanuatu */
    OS_WLAN_COUNTRY_VENEZUELA                                       = OS_MK_CNTRY( 'V', 'E', 0 ),             /* VE Venezuela */
    OS_WLAN_COUNTRY_VIET_NAM                                        = OS_MK_CNTRY( 'V', 'N', 0 ),             /* VN Viet_Nam */
    OS_WLAN_COUNTRY_VIRGIN_ISLANDS_BRITISH                          = OS_MK_CNTRY( 'V', 'G', 0 ),             /* VG Virgin_Islands,_British */
    OS_WLAN_COUNTRY_VIRGIN_ISLANDS_US                               = OS_MK_CNTRY( 'V', 'I', 0 ),             /* VI Virgin_Islands,_U.S. */
    OS_WLAN_COUNTRY_WALLIS_AND_FUTUNA                               = OS_MK_CNTRY( 'W', 'F', 0 ),             /* WF Wallis_and_Futuna */
    OS_WLAN_COUNTRY_WEST_BANK                                       = OS_MK_CNTRY( '0', 'C', 0 ),             /* 0C West_Bank */
    OS_WLAN_COUNTRY_WESTERN_SAHARA                                  = OS_MK_CNTRY( 'E', 'H', 0 ),             /* EH Western_Sahara */
    OS_WLAN_COUNTRY_WORLD_WIDE_XV_REV983                            = OS_MK_CNTRY( 'X', 'V', 983 ),           /* Worldwide Locale Revision 983 */
    OS_WLAN_COUNTRY_WORLD_WIDE_XX                                   = OS_MK_CNTRY( 'X', 'X', 0 ),             /* Worldwide Locale (passive Ch12-14) */
    OS_WLAN_COUNTRY_WORLD_WIDE_XX_REV17                             = OS_MK_CNTRY( 'X', 'X', 17 ),            /* Worldwide Locale (passive Ch12-14) Revision 17 */
    OS_WLAN_COUNTRY_YEMEN                                           = OS_MK_CNTRY( 'Y', 'E', 0 ),             /* YE Yemen */
    OS_WLAN_COUNTRY_ZAMBIA                                          = OS_MK_CNTRY( 'Z', 'M', 0 ),             /* ZM Zambia */
    OS_WLAN_COUNTRY_ZIMBABWE                                        = OS_MK_CNTRY( 'Z', 'W', 0 ),             /* ZW Zimbabwe */
} os_wlan_country_code_t;

#define ION_WLAN_NONE                   ION_WLAN(0)
#define ION_WLAN_AP_START               ION_WLAN(1)
#define ION_WLAN_AP_STOP                ION_WLAN(2)
#define ION_WLAN_STA_START              ION_WLAN(3)
#define ION_WLAN_STA_STOP               ION_WLAN(4)
#define ION_WLAN_AP_ASSOCIATED          ION_WLAN(5)
#define ION_WLAN_AP_DISASSOCIATED       ION_WLAN(6)
#define ION_WLAN_JOIN                   ION_WLAN(7)
#define ION_WLAN_LEAVE                  ION_WLAN(8)
#define ION_WLAN_SCAN                   ION_WLAN(9)

typedef enum
{
    OS_WLNA_LWIP
}os_wlan_protocol_type;

typedef enum
{
    OS_WLAN_NONE,
    OS_WLAN_STATION,
    OS_WLAN_AP,
    OS_WLAN_MODE_MAX
} os_wlan_mode_t;

typedef enum
{
    OS_802_11_BAND_5GHZ    = 0,          /* Denotes 5GHz radio band   */
    OS_802_11_BAND_2_4GHZ  = 1,          /* Denotes 2.4GHz radio band */
    OS_802_11_BAND_UNKNOWN = 0x7fffffff, /* unknown */
} os_wlan_802_11_band_t;

typedef enum
{
    OS_BSS_TYPE_INFRASTRUCTURE = 0,
    OS_BSS_TYPE_ADHOC          = 1,
    OS_BSS_TYPE_ANY            = 2,

    OS_BSS_TYPE_UNKNOWN        = -1
}os_wlan_bss_type_t;

struct os_wlan_ssid
{
    os_uint8_t len;
    char       val[OS_WLAN_SSID_MAX_LENGTH + 1];
};

typedef struct os_wlan_ssid os_wlan_ssid_t;

struct os_wlan_info
{
    const char             *ssid;
    const char             *password;
    os_wlan_security_t      security;
    os_wlan_country_code_t  country;
    os_uint8_t              channel;
};

struct os_wlan_scan_info
{
    os_wlan_ssid_t          ssid;
    os_uint8_t              bssid[6];
    os_int16_t              signal_strength;
    os_uint32_t             max_data_rate;
    os_wlan_bss_type_t      bss_type;
    os_wlan_security_t      security;
    os_uint8_t              channel;
    os_wlan_802_11_band_t   band;
    os_bool_t               on_channel;
};

struct os_wlan_scan_result
{
    struct os_wlan_scan_info   *scan_info;
    os_uint32_t                 num;
};

struct os_wlan_device;

struct os_wlan_device_ops
{
    os_err_t                    (*ap_start)(struct os_wlan_device *wlan_dev);
    os_err_t                    (*ap_stop)(struct os_wlan_device *wlan_dev);
    os_err_t                    (*sta_start)(struct os_wlan_device *wlan_dev);
    os_err_t                    (*sta_stop)(struct os_wlan_device *wlan_dev);
    os_err_t                    (*init)(struct os_wlan_device *wlan_dev);
    os_err_t                    (*get_mac)(struct os_wlan_device *wlan_dev, os_uint8_t *mac);
    os_err_t                    (*join)(struct os_wlan_device *wlan_dev);
    os_err_t                    (*check_join_status)(struct os_wlan_device *wlan_dev);
    os_err_t                    (*leave)(struct os_wlan_device *wlan_dev);
    void                        (*irq_handler)(void *param);
    os_err_t                    (*protocol_send)(struct os_wlan_device *wlan_dev, char *buff);
    struct os_wlan_scan_result *(*wlan_scan)(struct os_wlan_device *wlan, os_uint32_t msec);
    os_err_t                    (*wlan_scan_stop)(struct os_wlan_device *wlan);
    os_err_t                    (*wlan_scan_clean_result)(struct os_wlan_device *wlan, struct os_wlan_scan_result *info);
};

struct os_wlan_protocol
{
    os_err_t    (*init)(struct os_wlan_device *wlan_dev, struct os_wlan_protocol *protocol);
    os_err_t    (*event_handler)(struct os_wlan_device *wlan_dev, struct os_wlan_protocol *protocol, os_ubase_t event, os_ubase_t args);
    os_err_t    (*report)(struct os_wlan_device *wlan_dev, struct os_wlan_protocol *protocol, void *buff);
};

struct os_wlan_device
{
    struct os_device                    parent;
    const struct os_wlan_device_ops    *ops;
    struct os_wwd_device               *wwd_dev;
    struct os_wlan_protocol            *protocol[OS_WLAN_PROTOCOL_NUM_MAX];
    os_uint32_t                         flags;
    os_wlan_mode_t                      mode;
    struct os_wlan_info                 info;
};

void os_wlan_irq_handler(void *device);

os_err_t os_wlan_init_callback(struct os_wlan_device *wlan_dev);
os_err_t os_wlan_register(struct os_wlan_device *wlan_dev, const char *name);
os_err_t os_wlan_protocol_send(struct os_wlan_device *wlan_dev, char *buff);
os_err_t os_wlan_protocol_report(struct os_wlan_device *wlan_dev, char *buff);
os_err_t os_wlan_protocol_register(struct os_wlan_device *wlan_dev, struct os_wlan_protocol *protocol);

os_err_t os_wlan_ap_start(os_device_t *device, const char *ssid, const char *password, os_wlan_country_code_t country, os_wlan_security_t security, os_uint8_t channel);
os_err_t os_wlan_ap_stop(os_device_t *device);
os_err_t os_wlan_sta_start(os_device_t *device, os_wlan_country_code_t country, os_wlan_security_t security);
os_err_t os_wlan_sta_stop(os_device_t *device);

struct os_wlan_scan_result *os_wlan_scan(os_device_t *device, os_uint32_t msec);
os_err_t os_wlan_scan_clean_result(os_device_t *device, struct os_wlan_scan_result *info);
os_err_t os_wlan_scan_stop(os_device_t *device);

os_err_t os_wlan_get_mac(os_device_t *device, os_uint8_t *mac);
os_err_t os_wlan_join(os_device_t *device, const char *ssid, const char *password);
os_err_t os_wlan_leave(os_device_t *device);
os_err_t os_wlan_check_join_status(os_device_t *device);

#endif




