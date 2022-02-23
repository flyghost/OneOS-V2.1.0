#ifndef _AMS_H_
#define	_AMS_H_

#include "ams_port.h"
#include "ams_md5.h"



#define TAG "AMS_THREAD"


#define SOCKET_NULL 	0
#define SOCKET_ERR 		1
#define SOCKET_START 	2

enum {
	SOCKET_NONE,
	SOCKET_CREATE,
	SOCKET_CONNET,
};



#define LINE_END  "\n"
#define SEPARATOR ","

/**
 *********************************************************************************************************
 *						define list task
 *********************************************************************************************************
*/
#define AMS_NET_LIST_TASK_1		1
#define AMS_NET_LIST_TASK_2		2
#define AMS_NET_LIST_TASK_3		3

#define LIST_TASK_CREATE_SOCKET	7
#define LIST_TASK_CONNECT		8
#define LIST_TASK_REGISTER		9

#define LIST_TASK_PRIPORTY		1
#define NORMAL_TASK_PRIORITY	2

#define AMS_DOWNLOADAPP 10
#define AMS_STARTAPP 	11
#define AMS_RLISTAPP 	12
#define AMS_STOPAPP 	13
#define AMS_HEARTBEAT 	14
#define AMS_DELETEAPP 	15
#define AMS_RUNNGAPP 	16
#define AMS_RESETJVM 	17


enum {
	/*COMMAND CODE*/
	COMMAND_DOWNLOAD_APP = 30,
	COMMAND_START_APP    = 31,
	COMMAND_LIST_APP     = 32,
	COMMAND_STOP_APP     = 33,
	COMMAND_HEARTBEAT    = 34,
	COMMAND_DELETE_APP   = 35,
	COMMAND_RUNNING_APP  = 36,
	COMMAND_RESET_JVM    = 37,


	/*REPORT CODE*/
	RESPCODE_DOWNLOAD 	 = 60,
	RESPCODE_APPEXIST    = 61,
	RESPCODE_INSTALLOK 	 = 62,
	RESPCODE_APPSTARTOK  = 63,
	RESPCODE_HEARTBEAT 	 = 64,
	RESPCODE_REGISTER    = 65,
	RESPCODE_APPLIST 	 = 66,
	RESPCODE_APPFINISH 	 = 67,
	RESPCODE_DELETEOK 	 = 68,
	RESPCODE_RUNNINGAPPLIST = 69,


	
	/*ERROR REPORT CODE*/
	RESPCODE_FAIL_DOWNLOAD = 100,
	RESPCODE_INSTALLFAIL   = 101,
	RESPCODE_APPSTARTERROR = 102,
	RESPCODE_APPNOTEXIST   = 103,
	RESPCODE_DELETEFAIL    = 104,
	RESPCODE_APPNOTFINISH  = 105,
};


struct _ams_unpack_struct
{
    unsigned char filenum;
    unsigned char namelen;
    char name[256];
    unsigned int contentlen;
    char contentmd5[33];

};

#endif

