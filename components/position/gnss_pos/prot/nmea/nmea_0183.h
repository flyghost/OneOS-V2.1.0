/**
 ***********************************************************************************************************************
 * Copyright (c) China Mobile Communications Group Co.,Ltd.
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
 * @file        nmea_0183.h
 *
 * @brief       nmea0183 protocol parser head file
 *
 * @details
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-12  OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __NMEA_0183_H__
#define __NMEA_0183_H__

#include <os_types.h>
#include "onepos_common.h"

/* NMEA Config Start*/
/**
 ***********************************************************************************************************************
 * @enum        gnss_data_flag_t
 *
 * @brief       nmea sentence type
 ***********************************************************************************************************************
 */
typedef enum
{
    GNSS_RMC_DATA_FLAG = 0x01,
    GNSS_GGA_DATA_FLAG = 0x02,
    GNSS_GSV_DATA_FLAG = 0x04,
    GNSS_GLL_DATA_FLAG = 0x08,
    GNSS_GSA_DATA_FLAG = 0x10,
    GNSS_VTG_DATA_FLAG = 0x20,
    GNSS_ZDA_DATA_FLAG = 0x40,

    GNSS_ALL_DATA_FLAG = 0x7f
} gnss_data_flag_t;


/* Length of leading character at start of a statement, e.g. $GP */
#define NMEA_SENTENCE_START_LENGTH 3
/* Length of identification characters at start of a statement, e.g. GGA */
#define NMEA_SENTENCE_IDS_LENGHT 3
/* First character of sentence */
#define NMEA_SENTENCE_START_CHAR '$'
/* End character of sentence */
#define NMEA_SENTENCE_END_CHAR '\n'

#define PRO_SENTEN_START_CHAR '#'
/* Typical messages might be 11 to a maximum of 79 characters in length */
#define NMEA_MIN_LENGTH 11
#define NMEA_MAX_LENGTH 128

/* Generally require transmission no more often than once per second */
#define NMEA_MIN_INTERVAL 2
#define NMEA_MAX_INTERVAL (NMEA_MIN_INTERVAL * NMEA_SENTENCE_NUM)

// nmea sentence paraser using time 0.5s( testing )
#define NMEA_PARSER_TIME 0.5f

// Maximum supported statement length
#define NMEA_SENTENCE_CHARS_MAX_LEN (NMEA_MAX_LENGTH + NMEA_SENTENCE_START_LENGTH + NMEA_SENTENCE_IDS_LENGHT + 1)
// Minimum supported statement length
#define NMEA_SENTENCE_CHARS_MIN_LEN (NMEA_MIN_LENGTH + NMEA_SENTENCE_START_LENGTH + NMEA_SENTENCE_IDS_LENGHT + 1)
// Length of ring buff used to receive statememts
#define NMEA_MAX_BUFF_LEN (NMEA_SENTENCE_CHARS_MAX_LEN * 4)

/*
    Support for RMC data set (minimum specified GNSS data is recommended) contains information about time, longitude,
    latitude, system status, speed, heading, and date.All GNSS receivers broadcast this data set.
*/
#ifndef NMEA_SUPP_RMC
#define NMEA_SUPP_RMC 1
#endif

/*
    Support command GGA data set (nmea data) contains information about time, longitude and latitude, system quality,
    number of satellites used, and altitude.
*/
#ifndef NMEA_SUPP_GGA
#define NMEA_SUPP_GGA 1
#endif

/*
    The GSV data set (visible GNSS satellites) contains information about the number of visible satellites and their
    identification, elevation, azimuth, and signal-to-noise ratio.
*/
#ifndef NMEA_SUPP_GSV
#define NMEA_SUPP_GSV 1
/* Maximum number of satellites supported by the one SV statement */
#define MAX_SATE_NUM_OF_ONE_GSV 4
#define MAX_GSV_NUM             9
#endif
/*
    Support all satellites in the GSV data set to obtain detailed information;
    If not enabled, only the number of satellites will be resolved.
*/
#define NMEA_SUPP_GSV_SATES_INFO 1

/*
    Support GLL data set (geographic location - longitude/latitude) contains information about longitude, latitude,
   time, and health.
*/
#ifndef NMEA_SUPP_GLL
#define NMEA_SUPP_GLL 1
#endif

/*
    Support GSA data set (GNSS DOP and valid satellites) contains information about the measurement mode (2d or 3d),
    the number of satellites used to determine the position, and the measurement accuracy (DOP: precision factor).
*/
#ifndef NMEA_SUPP_GSA
#define NMEA_SUPP_GSA 1
/*
    Number of satellite Numbers used to calculate the position
*/
#define NMEA_MAX_GSA_ID_NUM 12
#endif

/*
    Support VTG data set (ground heading and ground speed) contain information about heading and speed.
*/
#ifndef NMEA_SUPP_VTG
#define NMEA_SUPP_VTG 1
#endif

/*
    Support ZDA data set (time and date) contains information about coordinated universal time, date, and local time.
*/
#ifndef NMEA_SUPP_ZDA
#define NMEA_SUPP_ZDA 1
#endif

// Additional information
/**
 ***********************************************************************************************************************
 * @enum        nmea_sentence_id_t
 *
 * @brief       nmea sentence index number
 ***********************************************************************************************************************
 */
typedef enum
{
    NMEA_INVILID_TYPE = -2,
    NMEA_UNKNOWN_TYPE = -1,
    NMEA_SENTENCE_RMC = 0,
    NMEA_SENTENCE_GGA,
    NMEA_SENTENCE_GSV,
    NMEA_SENTENCE_GLL,
    NMEA_SENTENCE_GSA,
    NMEA_SENTENCE_VTG,
    NMEA_SENTENCE_ZDA,
    // Additional information
    NMEA_SENTENCE_NUM,
} nmea_sentence_id_t;

// Positioning system Mode Indicator: supplement to Status
// A = Autonomous mode
// D = Differential mode
// E = Estimated (dead reckoning) mode
// M = Manual input mode
// S = Simulator mode
// N = Data not valid
// Indicator can only be A\D when Status is V
typedef enum
{
    NMEA_MODE_UNKNOWN = -1,
    NMEA_AUTO_MODE    = 0,    // Autonomous mode
    NMEA_DIFF_MODE    = 1,    // Differential mode
    NMEA_ESTIMATED_MODE,      // Estimated (dead reckoning) mode
    NMEA_MANUAL_IN_MODE,      // Manual input mode
    NMEA_SIMLA_MODE,          // Simulator mode
    NMEA_DATA_NOT_VALID,      // Data not valid
    NMEA_MODES_NUM
} nmea_pos_sys_mode_indoct;

typedef enum
{
    NMEA_STATUS_UNKNOWN  = -1,
    NMEA_STATUS_INVIALID = 0,    // Invalid for all values of Indicator mode except for
    NMEA_STATUS_AUTONOMOUS,      // Autonomous
    NMEA_STATUS_DIFF,            // Differential
    NMEA_STATUS_NUM
} nmea_status_t;

static const char NMEA_MODE_INDICAT[NMEA_MODES_NUM] = {'A', 'D', 'E', 'M', 'S', 'N'};
#define nmea_get_mode_num(pmode_c, pmode_n)                                                                            \
    do                                                                                                                 \
    {                                                                                                                  \
        int i = 0;                                                                                                     \
        for (i = 0; i < NMEA_MODES_NUM; i++)                                                                           \
        {                                                                                                              \
            if (*pmode_c == NMEA_MODE_INDICAT[i])                                                                      \
            {                                                                                                          \
                *pmode_n = (nmea_pos_sys_mode_indoct)i;                                                                \
                break;                                                                                                 \
            }                                                                                                          \
        }                                                                                                              \
        if (NMEA_MODES_NUM == i)                                                                                       \
            *pmode_n = NMEA_MODE_UNKNOWN;                                                                              \
    } while (0)

static const char NMEA_STATUS[NMEA_STATUS_NUM] = {'V', 'A', 'D'};
#define nmea_get_status_num(pstatus_c, pstatus_n)                                                                      \
    do                                                                                                                 \
    {                                                                                                                  \
        int i = 0;                                                                                                     \
        for (i = 0; i < NMEA_STATUS_NUM; i++)                                                                          \
        {                                                                                                              \
            if (*pstatus_c == NMEA_STATUS[i])                                                                          \
            {                                                                                                          \
                *pstatus_n = i;                                                                                        \
                break;                                                                                                 \
            }                                                                                                          \
        }                                                                                                              \
        if (NMEA_STATUS_NUM == i)                                                                                      \
            *pstatus_n = NMEA_STATUS_UNKNOWN;                                                                          \
    } while (0)
	
// RMC
// Recommended Minimum Specific GNSS Data page.106
typedef struct
{
    onepos_time_t              time;         /*  UTC of position fix  */
    int                        status;       /* Data status4, A = Data valid, V = Navigation receiver warning) */
    onepos_com_float_t         latitude;     /* Destination waypoint lat. - N/S */
    onepos_com_float_t         longitude;    /* Destination waypoint longitude, E/W */
    onepos_com_float_t         speed;        /*  Speed over ground, knots ） */
    onepos_com_float_t         course;       /* Course Over Ground, degrees True */
    onepos_date_t              date;         /* Date: ddmmyy */
    onepos_com_float_t         variation;    /*  Magnetic variation, degrees E/W */
    nmea_pos_sys_mode_indoct   mode_indicat; /*  Mode Indicator */
} nmea_rmc_t;

// GGA
// Global Positioning System Fix Data
// GGA quality:
// 0 = Fix not available or invalid
// 1 = GPS SPS Mode, fix valid
// 2 = Differential GPS, SPS Mode, fix valid
// 3 = GPS PPS Mode, fix valid
// 4 = Real Time Kinematic. System used in RTK mode with fixed integers
// 5 = Float RTK. Satellite system used in RTK mode, floating integers
// 6 = Estimated (dead reckoning) Mode
// 7 = Manual Input Mode
// 8 = Simulator Mode
typedef struct
{
    onepos_time_t  	   time;            /* UTC */
    onepos_com_float_t latitude;        /* Latitude, N/S */
    onepos_com_float_t longitude;       /*  Longitude, E/W */
    os_int32_t         quality;         /* Status */
    os_int32_t         satellites_used; /* Number of satellites in use, 00-12, may be different from the number in view */
    onepos_com_float_t hdop;            /* Horizontal dilution of precision */
    onepos_com_float_t altitude;        /* Altitude re: mean-sea-level (geoid) */
    char               altitude_units;  /* meters */
    onepos_com_float_t geoidal_separat; /*  Geoidal separation */
    char               geoidal_separa_units; /*  meters */
    onepos_com_float_t dgps_age;             /*  Age of Differential GPS data */
    os_int32_t   dgps_id;              /*  Differential reference station ID, 0000-1023 */
} nmea_gga_t;

// GSV
// GNSS Satellites In View
#if NMEA_SUPP_GSV_SATES_INFO
typedef struct
{
    os_int32_t  num;       /* Satellite ID number */
    os_int32_t  elevation; /*  Elevation, degrees, 90o maximum */
    os_int32_t  azimuth;   /* Azimuth, degrees True, 000 to 359 */
    char   snr;       /* SNR (C/No) 00-99 dB-Hz, null when not tracking */
} nmea_sate_t;
#endif

typedef struct
{
    os_int32_t total_msgs; /* Total number of sentences, 1 to 9 */
    os_int32_t msg_nr;     /* Sentence number, 1 to 9 */
    os_int32_t total_sats; /* Total number of satellites in view */
#if NMEA_SUPP_GSV_SATES_INFO
    nmea_sate_t sates_info[MAX_SATE_NUM_OF_ONE_GSV * MAX_GSV_NUM]; /* Satellite details information */
#endif
} nmea_gsv_t;

// GLL
// Geographic Position – Latitude/Longitude
typedef struct
{
    onepos_com_float_t       latitude;     /* Latitude, N/S */
    onepos_com_float_t       longitude;    /*  Longitude, E/W */
    onepos_time_t            time;         /*  UTC of position */
    os_int32_t               status;       /* Status */
    nmea_pos_sys_mode_indoct mode_indicat; /*  Mode Indicator */
} nmea_gll_t;

// GSA
// GNSS DOP and Active Satellites
typedef struct
{
    char    	       opera_calcu__mode;               /* Select Solution Mode */
    os_int32_t         calcu_mode;                      /* Solution Mode */
    os_int32_t   	   satells_id[NMEA_MAX_GSA_ID_NUM]; /* ID numbers1 of satellites used in solution */
    onepos_com_float_t pdop;                            /* PDOP */
    onepos_com_float_t hdop;                            /* hdop */
    onepos_com_float_t vdop;                            /* vdop */
} nmea_gsa_t;

// VTG
// Course Over Ground & Ground Speed
typedef struct
{
    nmea_pos_sys_mode_indoct mode_indoct;                  /*  Mode Indicator */
    onepos_com_float_t       course_over_ground_map;       /*  Course over ground */
    char                	 degree_true;                  /* degrees True T */
    onepos_com_float_t       course_over_ground_mangnetic; /* Course over ground, degrees Magnetic */
    char                	 degree_magnetic;
    onepos_com_float_t       speed_N;       			   /*  Speed over ground */
    char                	 speed_N_units; 		       /* knots */
    onepos_com_float_t       speed_K;                      /* Speed over ground */
    char                	 speed_K_units;                /* km/hr */
} nmea_vtg_t;

#ifdef NMEA_SUPP_ZDA
typedef struct
{
    onepos_time_t time;    /* UTC */
    onepos_date_t date;    /* date */
    os_int32_t    local_h; /* Local zone hours, 00 to ±13 hrs */
    os_int32_t    local_m; /* Local zone minutes, 00 to +59 */
} nmea_zda_t;
#endif

// Additional information

// NMEA data struct
typedef struct
{

#if NMEA_SUPP_RMC
    nmea_rmc_t rmc_frame;
#endif

#if NMEA_SUPP_GGA
    nmea_gga_t gga_frame;
#endif

#if NMEA_SUPP_GSV
    nmea_gsv_t gsv_frame;
#endif

#if NMEA_SUPP_GLL
    nmea_gll_t gll_frame;
#endif
#if NMEA_SUPP_GSA
    nmea_gsa_t gsa_frame;
#endif
#if NMEA_SUPP_VTG
    nmea_vtg_t vtg_frame;
#endif
#if NMEA_SUPP_ZDA
    nmea_zda_t zda_frame;
#endif
    // Additional information

    os_uint32_t valid_flag;
} nmea_t;

//  handle of nmea sentence parse function
typedef os_bool_t (*nmea_parser_func_t)(nmea_t *, const char *);

// to improve portability and flexibility, a string is used to represent the composition of a statement
// d -> director（N\E\S\W）
// c -> character
// f -> float
// i -> int
// s -> string
// D -> date
// T -> times
// _ -> ignore
// ; -> jump
typedef struct
{
    const char *  sentence_id_str;    // sentence id ,eg: RMC\GGA\...
    const char *  sentence_format;    // sentence format
    nmea_parser_func_t parse_fun;          // sentence parser function
} nmea_sentence_praser_t;

extern os_int32_t gnss_loca_start(void);
extern void display_nmea_pos_result(nmea_t *pos_data);
extern os_bool_t get_gnss_data(nmea_t *nmea, gnss_data_flag_t get_data_type);
extern os_bool_t nmea_prot_parse(const char *data, nmea_t* pos_data, os_size_t length);

/* NMEA Config End*/

#endif /* __NMEA_0183_H__ */
