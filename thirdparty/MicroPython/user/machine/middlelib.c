
#include "usr_misc.h"
#include <string.h>
#include "middlelib.h"
#include "mpconfigport.h"

int HexStr2Byte(char *buf, uint32_t bufsize, void *dest)
{
	uint8_t len=0, *temp = dest;
	int h_4bit, l_4bit;
	if (strlen((char *)buf) != bufsize || bufsize%2 != 0){
		mp_err("The length of data is error! \n");
		return MP_ERROR;
	}
	for (int i=0; i < bufsize; i+=2){
		h_4bit = char2int(buf[i]);
		l_4bit = char2int(buf[i+1]);
		if (h_4bit == MP_ERROR || l_4bit == MP_ERROR){
			return 0;
		}
		temp[len] = h_4bit << 4 |  l_4bit;
		len++;
	}
	return len;
}

