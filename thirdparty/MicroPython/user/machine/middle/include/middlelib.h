#ifndef __MIDDLELIB_H__
#define __MIDDLELIB_H__
#include <stdint.h>

/**
*********************************************************************************************************
* @brief                                 字节数据转化为字符串
*
* @description: 调用此函数，将字节数据转化为字符串。
*
* @param	  :	data		字节数据
*
*				buf			缓存区，存放结果
*
*				flag		补位标志，高位补0， 当目标为个位数时，此标志起作用
*
* @return	  :	转化结果
*
* @example    : byte2char(2, buf, 0)  >>> "2" 不补位
*				byte2char(2, buf, 1)  >>> "02" 补位
*********************************************************************************************************
*/
inline static char *byte2char(uint8_t data, char *buf, uint8_t flag)
{
	uint8_t h_4bit = data >> 4, l_4bit = data &0x0f;
	if (!flag && data < 10){
		buf[0]  = (l_4bit > 9)? (l_4bit - 10 + 'A'): (l_4bit + '0'); 
		return buf;
	}
	buf[0]  = (h_4bit > 9)? (h_4bit - 10 + 'A'): (h_4bit + '0'); 
	buf[1]  = (l_4bit > 9)? (l_4bit - 10 + 'A'): (l_4bit + '0'); 
	return buf;
}

inline static int char2int(uint8_t data)
{
	if(data <=  '9'){
		return  data - '0';
	} else if (data >=  'A' && data <= 'F'){
		return data - 'A' + 10;
	} else if  (data >=  'a' && data <= 'f'){	 
		return data - 'a'+ 10;
	}
	return -1;
}


int HexStr2Byte(char *buf, uint32_t bufsize, void *dest);

inline static int reverse_find_char(char * name, char c)
{
	int site = -1, i = 0;
	while( *(name+ i) != '\0'){
		if (*(name+ i) == c){
			site = i;
		}
		i++;
	}
	return site;
}

#endif
