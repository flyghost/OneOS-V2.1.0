#ifndef __ONEPOS_RTCM__
#define __ONEPOS_RTCM__

#include <onepos_common.h>

#define RTCM_BUFFER_MAX_LEN  GNSS_BUFFER_LEN_MAX
#define RTCM_MESSAGE_MAX_LEN (1029)
#define RTCM_MAX_NUM         (4)

#define RTCM_HEADER1      (0xd3)
#define RTCM_HEADER1MIX   (0xff)
#define RTCM_HEADER2      (0x00)
#define RTCM_HEADER2MIX   (0xfc)
#define RTCM_HEADER_LEN   (2)
#define RTCM_LEN_1_MIX    (0x03)
#define RTCM_CRC_HEAD_LEN (3)
#define RTCM_CRC_SELF_LEN (3)


typedef enum tag_rtcm_id
{
	RTCM_1005 = 1005,
	RTCM_1033 = 1033,
	RTCM_1074 = 1074,
	RTCM_1124 = 1124,
	RTCM_1075 = 1075,
	RTCM_1125 = 1125
}rtcm_id_t;

typedef  struct tag_onepos_rtcm_use
{
	rtcm_id_t       rtcm_id;
	os_uint16_t     rtcm_data_len;	
	os_uint16_t     heart_cnt;
	char      	   *rtcm_start_addr;
	os_uint8_t      is_new_flag;
}onepos_rtcm_use_t;


/*        function      */

extern os_int8_t rtcm_use_index(os_uint16_t rtcm_id);
void onepos_rtcm_init(onepos_rtcm_use_t *onepos_rtcm_use);
os_err_t onepos_rtcm_buffer_maitain(char *gnss_data_buffer , os_uint16_t buffer_len , char heart_cnt ,
	                            onepos_rtcm_use_t *onepos_rtcm_use ,
                                os_uint16_t *communication_error);
#endif
