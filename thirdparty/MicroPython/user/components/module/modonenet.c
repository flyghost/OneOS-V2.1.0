/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Damien P. George on behalf of Pycom Ltd
 * Copyright (c) 2017 Pycom Limited
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "modonenet.h"

#if MICROPY_PY_ONENET

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <os_memory.h>
#include "usr_timer.h"
#include "py/objlist.h"
#include "py/runtime.h"
#ifdef PKG_USING_SE
#include <log/elog.h>
//#include "nbconf.h"
#include "nbiot.h"
#include "SecuritySDKUtils.h"
#endif
// onenet class
typedef struct _onenet_obj_t {
    mp_obj_base_t base;
    uint32_t lifetime; //存活时间
    uint32_t insId; //实例id
    uint8_t res_count;//当前创建的资源个数
} onenet_obj_t;

STATIC const mp_obj_type_t onenet_type;
static bool nbinit_flag = 0;
static StNBIoTValue security_val[NBIOT_RESOURCE_SIZE] = {0};
static StNBIoTUri security_uri;



#if 1
/*
	OneNET消息接收链表为带尾指针的单向循环链表
*/
//msg为OneNET消息数据的结构类型
typedef struct onenet_msg_node
{
	NBIoT_LWM2M_Mode mode;
	uint32_t msgId;
	StNBIoTUri uri;
	uint8_t *msgdata;
	int msgdatalen;
}msg;

//plist为链表节点指针
typedef struct  onenet_msg_list
{
    struct  onenet_msg_list *next;
    msg data;
}*plist,listnode;

//定义尾部指针，以标记单链表尾结点
static plist ptail = NULL;

//分配一个链表节点空间，返回该节点地址
static plist malloc_node()
{
	plist node = (plist)os_malloc(sizeof(listnode));
	if(node == NULL)
	{
		os_kprintf("malloc_node fail");
		return NULL;
	}
	node->next = NULL;
	memset(&node->data, 0, sizeof(msg));
	return node;
}

//释放一个链表节点空间，若该节点data->msgdata有数据，也一起释放
static void free_node(plist node)
{
	if(node == NULL)
	{
		return;
	}
	if(node->data.msgdata != NULL)
	{
		os_free(node->data.msgdata);
		node->data.msgdata = NULL;
	}
	node->next = NULL;
	os_free(node);
	node = NULL;
	return;
}

//消息成员赋值,若有msg消息，则分配内存并赋值
static int msg_put(plist node,msg data)
{
	node->data.mode = data.mode;
	node->data.msgId = data.msgId;
	node->data.uri.insId = data.uri.insId;
	node->data.uri.objId = data.uri.objId;
	node->data.uri.resId = data.uri.resId;
	node->data.msgdatalen = data.msgdatalen;
	if(data.msgdata != NULL)
	{
		//int msglen = strlen(data.msgdata);
		int msglen = data.msgdatalen;
		node->data.msgdata = (uint8_t *)os_malloc(msglen+1);
		if(node->data.msgdata == NULL)
		{
			os_kprintf("malloc msg fail");
			return -1;
		}
		memset(node->data.msgdata, 0, msglen+1);
		memcpy(node->data.msgdata, data.msgdata, msglen);
	}
	return 0;
}


/*
	获取节点中的消息成员信息,若有msg消息，则为dataout的msg分配内存并赋值
	msg data的长度保存在msgdata_len中传出
	注意：dataout中的msg成员，若该节点中的msg有数据，则会在此处为dataout.msg赋值，外部读取后应予以释放
*/
static int msg_get(plist node,msg* dataout, int* msgdata_len)
{
	if(node == NULL)
	{
		return -1;
	}
	dataout->mode = node->data.mode;
	dataout->msgId = node->data.msgId;
	dataout->uri.insId = node->data.uri.insId;
	dataout->uri.objId = node->data.uri.objId;
	dataout->uri.resId = node->data.uri.resId;
	if(node->data.msgdata != NULL)
	{
		//int msglen = strlen(node->data.msgdata);
		int msglen = node->data.msgdatalen;
		dataout->msgdata = (uint8_t *)os_malloc(msglen+1);
		if(dataout->msgdata == NULL)
		{
			os_kprintf("malloc dataout msg fail");
			*msgdata_len = -1;
			return -1;
		}
		memset(dataout->msgdata, 0, msglen+1);
		memcpy(dataout->msgdata, node->data.msgdata, msglen);
		*msgdata_len = msglen;
	}
	else
	{
		dataout->msgdata = NULL;
		*msgdata_len = 0;
	}
	return 0;
}

static plist list_creat_node(msg data)
{
	plist node = malloc_node();
	if(node == NULL)
	{
		return NULL;
	}
	if(msg_put(node, data)<0)
	{
		free_node(node);
		return NULL;
	}
	return node;
}


//数据入队，尾部添加节点，传入尾指针及添加的节点，返回链表尾指针
static plist list_push_add(plist tail, plist node)
{
	if(tail == NULL)
	{
		tail = node;
		tail->next = tail;
	}
	else
	{
		node->next = tail->next;
		tail->next = node;
		tail = node;
	}
	return tail;
}


/*
	数据出队，删除头部节点，即尾部节点下一个节点，传入尾指针
	
*/
plist list_pop_del(plist tail)
{
	if(tail == NULL)
	{
		return tail;
	}
	plist headnode = tail->next;
	if(headnode == tail) //只有一个节点
	{
		free_node(tail);
		return NULL;
	}
	tail->next = headnode->next;
	headnode->next = NULL;
	free_node(headnode);
	return tail;
}

//获取链表总节点个数
int list_num()
{
	if(ptail == NULL)
	{
		return 0;
	}
	plist headnode = ptail->next;
	plist p = headnode;
	int len = 0;
	do
	{
		len++;
		p = p->next;
	}while(p != headnode);
	return len;
}

#endif

/*写入一条OneNET消息*/
static int onenet_msg_write(msg datain)
{
	plist node = list_creat_node(datain);
	if(node == NULL)
	{
		return -1;
	}
	ptail = list_push_add(ptail, node);
	return 1;
}

/*读取一条OneNET消息*/
static int onenet_msg_read(msg* dataout, int *datalen)
{
	plist node;
	if(ptail == NULL)
	{
		return 0;
	}
	node=ptail->next;
	if(msg_get(node,dataout, datalen) < 0)
	{
		return -1;
	}
	ptail = list_pop_del(ptail);
	return 1;
}


/*
*资源消息回调函数
*/
static void SecurityService_Callback(NBIoT_LWM2M_Mode mode, uint32_t msgId, StNBIoTUri uri, uint8_t *mesg)
{
	uint32_t len=0;
	//os_kprintf("recv onenet data mode = %d, msgId = %d\n", mode,msgId);
	//os_kprintf("uri.objId = %d, uri.insId= %d, uri.resId=%d", uri.objId,uri.insId,uri.resId);

	if(mesg != NULL)
	{
		//os_kprintf("recv onenet data %s\n", mesg);
		len = strlen((char *)mesg) / 2;
		util_str2hex(mesg, strlen((char *)mesg), mesg);

		log_d("data:");
		log_h((char *)mesg,len);
		log_ln();
	}

	msg msginfo;
	msginfo.mode = mode;
	msginfo.msgId = msgId;
	msginfo.uri.insId = uri.insId;
	msginfo.uri.objId = uri.objId;
	msginfo.uri.resId = uri.resId;
	msginfo.msgdata = mesg;
	msginfo.msgdatalen = len;
	onenet_msg_write(msginfo);
	//os_kprintf("recv onenet2 datalen %d\n", msginfo.msgdatalen);
	return;
}

STATIC mp_obj_t onenet_obj_init_helper(onenet_obj_t* self_in, size_t n_args, const mp_obj_t *args)
{
	mp_arg_check_num(n_args, 0, 0, 1, false);
	onenet_obj_t *self = self_in;
	uint32_t lifetime = 3000;
	uint32_t ret = 0;
	int cnt=0;
	if (n_args == 1) 
	{
		lifetime = mp_obj_get_int(args[0]);
		if( lifetime < 10 || lifetime > 84600)
		{
			mp_raise_ValueError("unsupported lifetime(10~84600) value!!!"); 
		}
	} 
	os_kprintf("new onenet lifetime = %d\n", lifetime);
	self->lifetime = lifetime;
	//nb模组初始化
	if(!nbinit_flag)
	{
		ret = NBIoT_Device_Init();
		if(ret != 0)
		{
			os_kprintf("init...\n");
			mp_hal_delay_ms(1000*5);
			do
			{
				if(cnt > 5)
				{
					break;
				}
				ret = NBIoT_Device_Init();
				mp_hal_delay_ms(1000);
				
				cnt ++;
			}
			while(ret != 0);
		}
		
		if(ret != 0)
		{
			os_kprintf("NBIoT_Device_Init ret = %d\n", ret);
			mp_raise_NotImplementedError("5311 init failed");
		}
		nbinit_flag = true;
	}

	//创建一个OneNet连接，设置资源消息回调函数
	if((int32_t)self->insId >= 0)
	{
		NBIoT_DisConnOneNet(self->insId);
		NBIoT_ClearOneNetDevice(self->insId);
	}
	ret = NBIoT_CreateOneNetDevice((uint8_t *)ONENET_COAP_SERVER, (uint8_t *)"", 31, lifetime, SecurityService_Callback);
	if((int32_t)ret < 0)
	{
	  mp_raise_ValueError("NBIoT_CreateOneNetDevice failed");
	}
	self->insId = ret;
	int rcount = self->res_count;
	//释放res data数据
	for(int i=0; i<rcount; i++)
	{
		if(security_val[i].data != NULL)
		{
			os_free(security_val[i].data);
			security_val[i].data = NULL;
		}
	}
	self->res_count = 0;
	os_kprintf("new onenet insId = %d", self->insId);
	return mp_const_none;

}

//OneNET.creat()
STATIC mp_obj_t onenet_obj_creat(size_t n_args, const mp_obj_t *args)
{
    mp_arg_check_num(n_args, 0, 0, 1, false);        

	onenet_obj_t *self = m_new_obj(onenet_obj_t);
	
	self->base.type = &onenet_type;
	self->insId = (uint32_t)(-1);
	self->res_count = 0;
	for(int i=0; i<NBIOT_RESOURCE_SIZE; i++)
	{
		security_val[i].data = NULL;
	}
    onenet_obj_init_helper(self, n_args, args);

    return (mp_obj_t) self;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(onenet_obj_creat_obj, 0, 1, onenet_obj_creat);

//OneNET.init()
STATIC mp_obj_t onenet_obj_init(size_t n_args, const mp_obj_t *args)
{
    return onenet_obj_init_helper(args[0],n_args-1,args+1);
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(onenet_obj_init_obj, 1, 2, onenet_obj_init);



//OneNET.deinit()
STATIC mp_obj_t onenet_deinit (mp_obj_t self_in)
{
    printf("onenet deinit function!\n");
	onenet_obj_t *self = self_in;
	if((int32_t)self->insId >= 0)
	{
		NBIoT_ClearOneNetDevice(self->insId);
	}
	
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(onenet_deinit_obj, onenet_deinit);

//OneNET.addresourse()
STATIC mp_obj_t onenet_addresource(mp_obj_t self_in, mp_obj_t url)
{
	int objid,resid;
	char *data;
	int datalen = 0;
	char *ins_data = NULL;
	mp_obj_type_t  *type = mp_obj_get_type(url);
	onenet_obj_t *self = self_in;
	if(self->res_count >= NBIOT_RESOURCE_SIZE)
	{
		os_kprintf("resource num is full\n");
		return mp_obj_new_bool(0);
	}
	int32_t ret = 0;
	if(type == &mp_type_dict)
	{
		objid = mp_obj_get_int(mp_obj_dict_get(url, mp_obj_new_str("objid", 5)));
		resid = mp_obj_get_int(mp_obj_dict_get(url, mp_obj_new_str("resid", 5)));
		data = (char *)mp_obj_str_get_str(mp_obj_dict_get(url, mp_obj_new_str("data", 4)));
		os_kprintf(" objid is %d, resid is %d, data is %s\n",objid, resid,data);

		security_uri.insId = self->insId;
		security_uri.objId = objid;
		security_uri.resId = resid;
		datalen = strlen(data);

		if(security_val[self->res_count].data != NULL)
		{
			os_free(security_val[self->res_count].data);
			security_val[self->res_count].data = NULL;
		}
		//在rt空间为data分配堆空间，待OneNET断开连接后释放该数据空间
		ins_data = (char *)os_malloc(datalen+1);
		if(ins_data == NULL)
		{
			os_kprintf("ins_data malloc fail\n");
			return mp_obj_new_bool(0);
		}
		//os_kprintf("rt_malloc res_val[%d]", self->res_count);
		memset(ins_data, 0, datalen+1);
		memcpy(ins_data, data, datalen);
		security_val[self->res_count].data = (uint8_t *)ins_data;
		security_val[self->res_count].dataType = TYPE_STRING;
		security_val[self->res_count].dataLen = datalen;
		
		ret = (int32_t)NBIoT_AddOneNetResourse(self->insId, security_uri, &security_val[self->res_count]);
		if(ret < 0)
		{
			//释放data空间
			if(security_val[self->res_count].data != NULL)
			{
				os_free(security_val[self->res_count].data);
				security_val[self->res_count].data = NULL;
			}
			
			os_kprintf("NBIoT_AddOneNetResourse ret = %d",ret);
		}
		self->res_count++;
		return mp_obj_new_bool((ret == 0));
	}
	else if(type == &mp_type_tuple)
	{
		size_t len;
		mp_obj_t *elem;
		int cur_count = self->res_count;
		mp_obj_get_array(url, &len, &elem);
		if(cur_count+len >= NBIOT_RESOURCE_SIZE)
		{
			os_kprintf("resource num too many\n");
			return mp_obj_new_bool(0);
		}
		for(int i=0; i<len; i++)
		{
			if(mp_obj_get_type(elem[i]) != &mp_type_dict)
			{
				mp_raise_TypeError("this must be dict");
			}
			objid = mp_obj_get_int(mp_obj_dict_get(elem[i], mp_obj_new_str("objid", 5)));
			resid = mp_obj_get_int(mp_obj_dict_get(elem[i], mp_obj_new_str("resid", 5)));
			data = (char *)mp_obj_str_get_str(mp_obj_dict_get(elem[i], mp_obj_new_str("data", 4)));
			os_kprintf(" objid is %d, resid is %d, data is %s\n",objid, resid,data);

			security_uri.insId = self->insId;
			security_uri.objId = objid;
			security_uri.resId = resid;


			datalen = strlen(data);
			if(security_val[self->res_count].data != NULL)
			{
				os_free(security_val[self->res_count].data);
				security_val[self->res_count].data = NULL;
			}
			//在rt空间为data分配堆空间，待OneNET断开连接后释放该数据空间
			ins_data = (char *)os_malloc(datalen+1);
			if(ins_data == NULL)
			{
				os_kprintf("ins_data malloc fail\n");
				//to do 此处应增加删除之前订阅的流程，并释放data空间
				return mp_obj_new_bool(0);
			}
			//os_kprintf("rt_malloc res_val[%d]", cur_count+i);
			memset(ins_data, 0, datalen+1);
			memcpy(ins_data, data, datalen);
			security_val[cur_count+i].data = (uint8_t *)ins_data;
			security_val[cur_count+i].dataType = TYPE_STRING;
			security_val[cur_count+i].dataLen = datalen;

			ret = (int32_t)NBIoT_AddOneNetResourse(self->insId, security_uri, &security_val[cur_count+i]);
			if(ret < 0)
			{
				//to do 此处应增加删除订阅的流程，并释放data空间
				os_kprintf("NBIoT_AddOneNetResourse ret = %d",ret);
				break;
			}
		}
		return mp_obj_new_bool((ret == 0));
	}
	else
	{
		mp_raise_TypeError("this must be tuple/dict");
	}
	
	return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(onenet_addresource_obj, onenet_addresource);


//OneNET.connect()
STATIC mp_obj_t onenet_connect(size_t n_args, const mp_obj_t *args)
{
	mp_arg_check_num(n_args, 0, 1, 2, false);
	onenet_obj_t *self = (onenet_obj_t*)MP_OBJ_TO_PTR(args[0]);
	if((int32_t)self->insId < 0)
	{
		os_kprintf("insid not init,should init first\n");
		return mp_obj_new_bool(0);
	}
	if(self->res_count <= 0)
	{
		os_kprintf("resourceID not add,should add first\n");
		return mp_obj_new_bool(0);
	}
	int timeout = 30;
	int cnt = 0;
	uint32_t ret = 0;
	if (n_args == 2) 
	{
		timeout = mp_obj_get_int(args[1]);
	} 
		
	do
	{
		if(cnt > 0)
		{
			os_kprintf("Retry Login,set timeout = %d\n", timeout);
		}
		if(cnt > 5)
		{
			break;
		}
		ret = NBIoT_ConnOneNet(self->insId, timeout);
		cnt ++;
	}
	while(ret != 0);
	
	if(ret != 0)
	{
		os_kprintf("Login time out\r\n");
		os_kprintf("Login Fail\r\n");
		NBIoT_DisConnOneNet(self->insId);
		NBIoT_ClearOneNetDevice(self->insId);
		self->insId = (uint32_t)(-1);
	}
	os_kprintf("onenet_connect ret = %d", ret);
	return mp_obj_new_bool((ret == 0));

}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(onenet_connect_obj, 1, 2, onenet_connect);


//OneNET.disconnect()
STATIC mp_obj_t onenet_disconnect(mp_obj_t self_in)
{

	onenet_obj_t *self = (onenet_obj_t*)self_in;
	uint32_t ret = NBIoT_DisConnOneNet(self->insId);
	os_kprintf("onenet_disconnect ret = %d", ret);
	if(ret == 0)
	{
		int res_count = self->res_count;
		//释放res data数据
		for(int i=0; i<res_count; i++)
		{
			//os_kprintf("free res_val[%d]", i);
			if(security_val[i].data != NULL)
			{
				os_free(security_val[i].data);
				security_val[i].data = NULL;
			}
		}
		self->res_count = 0;
		NBIoT_ClearOneNetDevice(self->insId);
	}
	return mp_obj_new_bool((ret == 0));

}
MP_DEFINE_CONST_FUN_OBJ_1(onenet_disconnect_obj, onenet_disconnect);


//OneNET.connectstate() 暂不支持
STATIC mp_obj_t onenet_connectstate(mp_obj_t self_in)
{

	#if 0
	onenet_obj_t *self = (onenet_obj_t*)self_in;
	
	int32_t ret = (int32_t)NBIoT_isLoginOneNET(self->insId);
	os_kprintf("NBIoT_isLoginOneNET ret = %d", ret);
	if(ret == 0)
	{
		//已登录
		ret = 2;
	}
	else if(ret == -3)
	{
		//正在登录中
		ret =1;
	}
	else
	{
		ret = -1;
	}
	return mp_obj_new_int(ret);
	#endif
	return mp_obj_new_int(0);
}
MP_DEFINE_CONST_FUN_OBJ_1(onenet_connectstate_obj, onenet_connectstate);


//OneNET.send(uri)
STATIC mp_obj_t onenet_send(mp_obj_t self_in, mp_obj_t url)
{

	int objid,resid;
	char *data;
	StNBIoTValue security_val;
	StNBIoTUri security_uri;
	mp_obj_type_t  *type = mp_obj_get_type(url);
	onenet_obj_t *self = self_in;
	int32_t ret = 0;
	if(type != &mp_type_dict)
	{
		mp_raise_TypeError("this must be tuple/dict");
	}
	
	
	objid = mp_obj_get_int(mp_obj_dict_get(url, mp_obj_new_str("objid", 5)));
	resid = mp_obj_get_int(mp_obj_dict_get(url, mp_obj_new_str("resid", 5)));
	data = (char *)mp_obj_str_get_str(mp_obj_dict_get(url, mp_obj_new_str("data", 4)));
	os_kprintf(" objid is %d, resid is %d, data is %s\n",objid, resid,data);

	security_uri.insId = self->insId;
	security_uri.objId = objid;
	security_uri.resId = resid;

	security_val.data = (uint8_t *)data;
	security_val.dataType = TYPE_STRING;

	ret = (int32_t)NBIoT_NotifyOneNet(self->insId, security_uri, &security_val);
	if(ret < 0)
	{
		os_kprintf("NBIoT_NotifyOneNet ret = %d",ret);
	}
	return mp_obj_new_bool((ret == 0));	
}
MP_DEFINE_CONST_FUN_OBJ_2(onenet_send_obj, onenet_send);


//OneNET.recv(timeout=0)
STATIC mp_obj_t onenet_recv(size_t n_args, const mp_obj_t *args)
{
	mp_arg_check_num(n_args, 0, 1, 2, false);
	//onenet_obj_t *self = (onenet_obj_t*)MP_OBJ_TO_PTR(args[0]);
	
	msg dataout;
	int datalen = 0;
	int ret = 0;
	
	int recv_num = 0;
	int hasnum = list_num();
	if(hasnum == 0)
	{
		return mp_const_none;
	}
	if (n_args == 2) 
	{
		recv_num = mp_obj_get_int(args[1]);
	}
	else
	{
		recv_num = hasnum;
	}
	recv_num = (recv_num>hasnum)?hasnum:recv_num;
	if(recv_num <= 0)
	{
		return mp_const_none;
	}

	//接收一个数据，返回为字典
	if(recv_num == 1)
	{
		ret = onenet_msg_read(&dataout, &datalen);
		//os_kprintf("dataout datalen = %d\n", datalen);
		if(ret > 0)
		{
#if 0
			os_kprintf("dataout.insId = %d\n", dataout.uri.insId);
			os_kprintf("dataout.objId = %d\n", dataout.uri.objId);
			os_kprintf("dataout.resId = %d\n", dataout.uri.resId);
			os_kprintf("dataout mode = %d\n", dataout.mode);
			os_kprintf("dataout msgId = %d\n", dataout.msgId);
			os_kprintf("dataout len = %d\n", datalen);
#endif
			mp_obj_t info = mp_obj_new_dict(5);
			mp_obj_dict_store(info, mp_obj_new_str("mode",4),  mp_obj_new_int(dataout.mode));
			mp_obj_dict_store(info, mp_obj_new_str("msgId",5),  mp_obj_new_int(dataout.msgId));
			mp_obj_dict_store(info, mp_obj_new_str("objId",5),  mp_obj_new_int(dataout.uri.objId));
			mp_obj_dict_store(info, mp_obj_new_str("resId",5),  mp_obj_new_int(dataout.uri.resId));
			if(dataout.msgdata != NULL)
			{
				mp_obj_dict_store(info, mp_obj_new_str("data",4),  mp_obj_new_bytearray(datalen,dataout.msgdata));
				os_free(dataout.msgdata);
				dataout.msgdata = NULL;
			}
			else
			{
				mp_obj_dict_store(info, mp_obj_new_str("data",4),   MP_OBJ_NEW_QSTR(MP_QSTRnull));
			}
			return info;
		}
		else
		{
			return mp_const_none;
		}
	}
	else//返回字典元组
	{
		mp_obj_t tuple[recv_num];
		int i=0;
		for(i=0; i<recv_num; i++)
		{
			ret = onenet_msg_read(&dataout, &datalen);
			if(ret > 0)
			{
				#if 0
				os_kprintf("dataout.insId = %d\n", dataout.uri.insId);
				os_kprintf("dataout.objId = %d\n", dataout.uri.objId);
				os_kprintf("dataout.resId = %d\n", dataout.uri.resId);
				os_kprintf("dataout mode = %d\n", dataout.mode);
				os_kprintf("dataout msgId = %d\n", dataout.msgId);
				os_kprintf("dataout len = %d\n", datalen);
				#endif
				tuple[i] = mp_obj_new_dict(5);
				mp_obj_dict_store(tuple[i], mp_obj_new_str("mode",4),  mp_obj_new_int(dataout.mode));
				mp_obj_dict_store(tuple[i], mp_obj_new_str("msgId",5),  mp_obj_new_int(dataout.msgId));
				mp_obj_dict_store(tuple[i], mp_obj_new_str("objId",5),  mp_obj_new_int(dataout.uri.objId));
				mp_obj_dict_store(tuple[i], mp_obj_new_str("resId",5),  mp_obj_new_int(dataout.uri.resId));
				if(dataout.msgdata != NULL)
				{
					os_kprintf("dataout msgdata = %s\n", dataout.msgdata);
					mp_obj_dict_store(tuple[i], mp_obj_new_str("data",4), mp_obj_new_bytearray(datalen,dataout.msgdata));
					os_free(dataout.msgdata);
					dataout.msgdata = NULL;
				}
				else
				{
					mp_obj_dict_store(tuple[i], mp_obj_new_str("data",4),  MP_OBJ_NEW_QSTR(MP_QSTRnull));
				}
			}
			else
			{
				break;
			}
		}
		return mp_obj_new_tuple(i+1, tuple);
	}
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(onenet_recv_obj, 1, 2, onenet_recv);


//OneNET.reset()
STATIC mp_obj_t onenet_reset(mp_obj_t self_in)
{
	
	return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(onenet_reset_obj, onenet_reset);


//OneNET.authinfo()
STATIC mp_obj_t onenet_authinfo(mp_obj_t self_in)
{
	char IMEI[20] = {0};
	char IMSI[20] = {0};
	
	if(NBIoT_Device_GetIMEI((uint8_t *)IMEI) == 0 && NBIoT_Device_GetIMSI((uint8_t *)IMSI) == 0)
	{
		mp_obj_t info = mp_obj_new_dict(2);
		mp_obj_dict_store(info, mp_obj_new_str("IMEI",4), mp_obj_new_str(IMEI,strlen(IMEI)));
        mp_obj_dict_store(info, mp_obj_new_str("IMSI",4), mp_obj_new_str(IMSI,strlen(IMSI)));
		return info;
	}
	else
		return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(onenet_authinfo_obj, onenet_authinfo);

//OneNET.PSM()
STATIC mp_obj_t onenet_psm(mp_obj_t self_in,mp_obj_t state)
{
	int psm_state = mp_obj_get_int(state);
	if(psm_state)
	{
		NBIoT_Device_OpenPSM();
	}
	else
	{
		NBIoT_Device_ClosePSM();
	}
	return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(onenet_psm_obj, onenet_psm);




//OneNET.modelname()
STATIC mp_obj_t onenet_modelname(mp_obj_t self_in)
{
	/*mem test*/
	//extern void list_mem(void);
	//list_mem();
	return mp_obj_new_str("M5311", 5);
}
MP_DEFINE_CONST_FUN_OBJ_1(onenet_modelname_obj, onenet_modelname);


STATIC const mp_rom_map_elem_t onenet_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&onenet_obj_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&onenet_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_addresource), MP_ROM_PTR(&onenet_addresource_obj) },
    { MP_ROM_QSTR(MP_QSTR_connect), MP_ROM_PTR(&onenet_connect_obj) },
    { MP_ROM_QSTR(MP_QSTR_disconnect), MP_ROM_PTR(&onenet_disconnect_obj) },
    { MP_ROM_QSTR(MP_QSTR_connectstate), MP_ROM_PTR(&onenet_connectstate_obj) },
	{ MP_ROM_QSTR(MP_QSTR_send), MP_ROM_PTR(&onenet_send_obj) },
    { MP_ROM_QSTR(MP_QSTR_recv), MP_ROM_PTR(&onenet_recv_obj) },
    { MP_ROM_QSTR(MP_QSTR_reset), MP_ROM_PTR(&onenet_reset_obj) },
    { MP_ROM_QSTR(MP_QSTR_authinfo), MP_ROM_PTR(&onenet_authinfo_obj) },
	{ MP_ROM_QSTR(MP_QSTR_psm), MP_ROM_PTR(&onenet_psm_obj) },	
    { MP_ROM_QSTR(MP_QSTR_modelname), MP_ROM_PTR(&onenet_modelname_obj) },
};
STATIC MP_DEFINE_CONST_DICT(onenet_locals_dict, onenet_locals_dict_table);


STATIC const mp_obj_type_t onenet_type = {
    { &mp_type_type },
    .name = MP_QSTR_onenet,
    .locals_dict = (mp_obj_t)&onenet_locals_dict,
};


STATIC const mp_rom_map_elem_t mp_module_onenet_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_OneNET) },
    { MP_ROM_QSTR(MP_QSTR_creat), MP_ROM_PTR(&onenet_obj_creat_obj) },

};

STATIC MP_DEFINE_CONST_DICT(mp_module_onenet_globals, mp_module_onenet_globals_table);

const mp_obj_module_t mp_module_onenet = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_onenet_globals,
};



/*
for test
*/
void onenetread(int argc, char**argv)
{
	
	int datalen = strlen(argv[1]);
	int len = 0;
	for(int i=0; i<datalen; i++)
	{
		len *= 10;
		len += argv[1][i]-'0';
	}
	os_kprintf("len : %d\n", len);
	msg dataout;
	int recvlen = 0;
	plist node;
	
	for(int i=0; i<len; i++)
	{
		if(ptail == NULL)
		{
			return;
		}
		node=ptail->next;
		if(msg_get(node,&dataout, &recvlen) < 0)
		{
			return;
		}
		os_kprintf("dataout.insId = %d\n", dataout.uri.insId);
		os_kprintf("dataout.objId = %d\n", dataout.uri.objId);
		os_kprintf("dataout.resId = %d\n", dataout.uri.resId);
		os_kprintf("dataout mode = %d\n", dataout.mode);
		os_kprintf("dataout msgId = %d\n", dataout.msgId);
		os_kprintf("dataout len = %d\n", recvlen);
		if(dataout.msgdata != NULL)
		{
			os_kprintf("dataout msgdata = %s\n", dataout.msgdata);
			os_free(dataout.msgdata);
			dataout.msgdata = NULL;
		}
		ptail = list_pop_del(ptail);
	}
	return;
}
MSH_CMD_EXPORT(onenetread, onenetread test)

void onenetwrite(int argc, char**argv)
{
	if(argc < 3)
	{
		os_kprintf("least have 2 param");
		return;
	}
	int datalen = strlen(argv[1]);
	int param1 = 0;
	int param2 = 0;
	for(int i=0; i<datalen; i++)
	{
		param1 *= 10;
		param1 += argv[1][i]-'0';
	}
	os_kprintf("param1 : %d\n", param1);


	datalen = strlen(argv[2]);
	for(int i=0; i<datalen; i++)
	{
		param2 *= 10;
		param2 += argv[2][i]-'0';
	}
	os_kprintf("param2 : %d\n",param2);
	
	msg msginfo;
	msginfo.mode = (NBIoT_LWM2M_Mode)param1;
	msginfo.msgId = param2;
	msginfo.uri.insId = param2+1;
	msginfo.uri.objId = param2+2;
	msginfo.uri.resId = param2+3;
	msginfo.msgdata = NULL;
	if(argc >= 4)
	{
		os_kprintf("datastr : %s\n",argv[3]);
		msginfo.msgdata = (uint8_t *)argv[3];
	}
	else
	{
		os_kprintf("datastr NULL\n");
	}
	plist node = list_creat_node(msginfo);
	if(node == NULL)
	{
		return;
	}
	ptail = list_push_add(ptail, node);
	return;
}
MSH_CMD_EXPORT(onenetwrite, onenetwrite test)
	
void onenetdata()
{
	if(ptail == NULL)
	{
		return;
	}
	plist headnode = ptail->next;
	plist p = headnode;
	do
	{
		os_kprintf("************\n");
		os_kprintf("uri.insId = %d\n", p->data.uri.insId);
		os_kprintf("uri.objId = %d\n", p->data.uri.objId);
		os_kprintf("uri.resId = %d\n", p->data.uri.resId);
		os_kprintf("mode = %d\n", p->data.mode);
		os_kprintf("msgId = %d\n", p->data.msgId);
		if(p->data.msgdata != NULL)
			os_kprintf("msgdata = %s\n", p->data.msgdata);
		p = p->next;
	}while(p != headnode);
}
MSH_CMD_EXPORT(onenetdata, onenetdata test)


#endif

