# **MoLink API**

## **简介**

MoLink组件[``<mo_api.h>``](../api/include/mo_api.h)提供了OneOS组件中的基础功能。通过将无线通信模组抽象为mo对象（模组对象），向用户层提供简洁易用的操作接口。同时MoLink组件设计兼容了通信模组的OpenCPU开发模式，极大提升了用户程序的可移植性，应用程序的无线连网功能可在AT模式和OpenCPU模式下无缝切换。后期MoLink组件将适配数量众多的无线通信模组，这样用户可以根据实际需求，便捷选择模组型号，轻松配置进行切换。

------

> ## **目录**
>
>> - [**MoLink API**](#molink-api)
>>   - [**简介**](#简介)
>>   - [**目录**](#目录)
>>   - [**一、模组实例管理接口**](#一模组实例管理接口)
>>     - [**1.1 模组创建及销毁**](#11-模组创建及销毁)
>>     - [**1.2 模组创建及销毁**](#12-模组创建及销毁)
>>     - [**1.3 模组管理**](#13-模组管理)
>>     - [**1.4 使用示例**](#14-使用示例)
>>   - [**二、通用控制接口**](#二通用控制接口)
>>     - [**2.1 通用控制接口说明**](#21-通用控制接口说明)
>>     - [**2.2 使用示例**](#22-使用示例)
>>   - [**三、网络服务接口**](#三网络服务接口)
>>     - [**3.1 网络服务接口说明**](#31-网络服务接口说明)
>>     - [**3.2 使用示例**](#32-使用示例)
>>   - [**四、套接字接口**](#四套接字接口)
>>     - [**4.1 套接字接口说明**](#41-套接字接口说明)
>>     - [**4.2 常见问题**](#42-常见问题)

------

## **一、模组实例管理接口**

模组的管理基于模组实例管理框架，由统一管理接口控制，用户可以不必再关心冗杂的模组的AT指令收发及解析，调用MoLink API轻松实现模组管理及具体业务。

### **1.1 模组创建及销毁**

用户使用MoLink组件，仅仅需要拥有一个模组实例，凭此接入MoLink组件，即可使用其所提供的各项功能。模组实例可由可视化工具配置后自动生成，亦可灵活手动创建，给与开发者充分灵活的发挥的空间。

<details open><summary>
<b>模组实例相关接口总览</b>
</summary>

| **接口**       | **说明**             |
| :------------- | :------------------- |
| mo_create      | 创建模组对象         |
| mo_destroy     | 销毁模组对象         |
| mo_get_by_name | 根据名称获取模组对象 |
| mo_get_default | 获取默认模组对象     |
| mo_set_default | 设置默认模组对象     |

</details>

------

### **1.2 模组创建及销毁**

MoLink提供自动创建和手动创建两种模组创建方式。用户可根据设备及具体应用场景进行选择。

#### 1.2.1 自动创建

<details open><summary>
<b>自动创建方式介绍</b>
</summary>

>自动创建方式：使用oneos-cube可视化配置工具menuconfig，在``(Top) → Components→ Network→ Molink``路径下，使能物联网模组支持功能``（[*] Enable IoT modules support）``，在此目录下，选择使能模组及配置是否自动创建模组。
>
>如图 以M5310-A为例：
>
>1) 在menuconfig中进入具体模组配置目录``(Top) → Components→ Network→ Molink→ Enable IoT modules support → Module→ NB-IOT Modules Support→ M5310A → M5310A Config``
>
>2) 使能自动创建功能``[*] Enable M5310A Module Object Auto Create``
>
>3) 在使能自动创建后会出现次级配置选项，配置好模组信息如：设备接口名``Interface Device Name``、模组波特率``Interface Device Rate``和AT指令最大接收长度``The maximum length of AT command data accepted``。保存配置后编译烧录即可在OneOS运行时自动创建模组。
>
>*) 注意：使用自动创建需关注模组在自动创建时是否正常工作，若模组未开机或工作状态不正常，不能使用自动创建功能。具体使用方法见[Molink-模组连接套件:图形化配置](../README.md)

```log
[*] Enable M5310A Module Object Auto Create            <-------使能模组自动创建功能
(uart2) M5310A Interface Device Name                   <-------接口名
(115200) M5310A Interface Device Rate                  <-------模组波特率
(512)   The maximum length of AT command data accepted <-------单条AT指令最大接收长度
-*- Enable M5310A Module General Operates              <-------使能通用控制接口
-*- Enable M5310A Module Network Service Operates      <-------使能网络服务接口
[*] Enable M5310A Module Ping Operates                 <-------使能ping功能接口
[*] Enable M5310A Module Ifconfig Operates             <-------使能ifconfig接口
[*] Enable M5310A Module Network TCP/IP Operates       <-------使能TCP/IP功能接口
[ ] Enable M5310A Module BSD Socket Operates           <-------使能BSD套接字
[*] Enable M5310A Module Onenet Nb Operates            <-------使能OneNetNB平台接口
```

</details>

#### 1.2.2 手动创建

<details open><summary>
<b>mo_create</b>
</summary>

MoLink也提供了手动创建模组实例功能，方便更加灵活管理模组。接口如下：

##### mo_create

该函数用于创建模组对象实例，其函数原型如下：

```c
mo_object_t *mo_create(const char *name, mo_type_t type, void *parser_config);
```

| **参数**      | **说明**                                      |
| :------------ | :-------------------------------------------- |
| name          | 模组名称                                      |
| type          | 模组型号                                      |
| parser_config | AT解析器参数结构体指针，OpenCPU架构此参数为空 |
| **返回**      | **说明**                                      |
| OS_NULL       | 创建失败                                      |
| 非OS_NULL     | 模组对象指针                                  |

*) 进行手动创建请勿使能此模组的自动创建功能。

</details>

#### 1.2.3 销毁

<details open><summary>
<b>mo_destroy</b>
</summary>

##### mo_destroy

该函数用于销毁模组对象实例，其函数原型如下：

```c
os_err_t mo_destroy(mo_object_t *self, mo_type_t type);
```

| **参数** | **说明**       |
| :------- | :------------- |
| self     | 模组对象       |
| type     | 支持的模组型号 |
| **返回** | **说明**       |
| OS\_EOK  | 成功           |

</details>

------

### **1.3 模组管理**

<details open><summary>
<b>APIs</b>
</summary>

##### mo_get_by_name

该函数用于根据名称获取模组对象，函数原型如下：

```c
mo_object_t *mo_get_by_name(const char *name);
```

| **参数**  | **说明**     |
| :-------- | :----------- |
| name      | 模组对象名称 |
| **返回**  | **说明**     |
| OS_NULL   | 获取失败     |
| 非OS_NULL | 模组对象指针 |

##### mo_get_default

该函数用于获取默认模组对象，函数原型如下：

```c
mo_object_t *mo_get_default(void);
```

| **参数**  | **说明**         |
| :-------- | :--------------- |
| 无        | 无               |
| **返回**  | **说明**         |
| OS_NULL   | 获取失败         |
| 非OS_NULL | 默认模组对象指针 |

##### mo_set_defaults

该函数用于设置默认模组对象，其函数原型如下：

```c
void mo_set_default(mo_object_t *self);
```

| **参数** | **说明** |
| :------- | :------- |
| self     | 模组对象 |
| **返回** | **说明** |
| 无       | 无       |

</details>

------

### **1.4 使用示例**

<details><summary>
<b>Samples</b>
</summary>

```c
#define RECV_BUF_LEN     (512)
#define TEST_MODULE_NAME "gm190"

/* manually create module */
mo_object_t *test_module = OS_NULL;
mo_object_t *temp_module = OS_NULL;
os_err_t     result      = OS_ERROR;

mo_parser_config_t parser_config = {.parser_name   = TEST_MODULE_NAME,
                                    .parser_device = test_device,
                                    .recv_buff_len = RECV_BUF_LEN};

test_module = mo_create("gm190", MODULE_TYPE_GM190, &parser_config);
OS_ASSERT(OS_NULL != test_module);

/* set default module instance */
#if 0
/* auto set default when create, but also you can call this func */
result = mo_set_default(test_module);
OS_ASSERT(OS_ERROR != result);
#endif

/* get default module instance */
temp_module = mo_get_default();
OS_ASSERT(OS_NULL == temp_module);

/* get module instance by name */
temp_module = mo_get_by_name(TEST_MODULE_NAME);
OS_ASSERT(test_module == temp_module);

/* destroy module */
result = mo_destroy(test_module, MODULE_TYPE_GM190);
OS_ASSERT(OS_EOK == result);
```

</details>

------

## **二、通用控制接口**

通用控制接口提供模组相关基本信息及功能查询设置，模组创建后，按需调用即可。

<details open><summary>
<b>通用控制接口总览</b>
</summary>

| **接口**                     | **说明**                   |
| :--------------------------- | :------------------------- |
| mo_at_test                   | 测试AT指令                 |
| mo_get_imei                  | 获取IMEI                   |
| mo_get_imsi                  | 获取IMSI                   |
| mo_get_iccid                 | 获取iccid                  |
| mo_get_cfun                  | 获取射频模式               |
| mo_set_cfun                  | 设置射频模式               |
| mo_get_firmware_version      | 获取模组固件版本信息       |
| mo_get_firmware_version_free | 释放获取的模组固件版本信息 |
| mo_get_eid                   | 获取SIM eID                |

</details>

------

### **2.1 通用控制接口说明**

<details open><summary>
<b>APIs</b>
</summary>

##### mo_at_test

该函数用于发送AT测试命令，其函数原型如下：

```c
os_err_t mo_at_test(mo_object_t *self);
```

| **参数**  | **说明** |
| :-------- | :------- |
| self      | 模组对象 |
| **返回**  | **说明** |
| OS\_EOK   | 成功     |
| 非OS\_EOK | 失败     |

##### mo_get_imei

该函数用于获取IMEI，该函数原型如下：

```c
os_err_t mo_get_imei(mo_object_t *self, char *value, os_size_t len);
```

| **参数**  | **说明**          |
| :-------- | :---------------- |
| self      | 模组对象          |
| value     | 存储IMEI的buf     |
| len       | 存储IMEI的buf长度 |
| **返回**  | **说明**          |
| OS\_EOK   | 成功              |
| 非OS\_EOK | 失败              |

##### mo_get_imsi

该函数用于获取IMSI，其函数原型如下：

```c
os_err_t mo_get_imsi(mo_object_t *self, char *value, os_size_t len);
```

| **参数**  | **说明**          |
| :-------- | :---------------- |
| self      | 模组对象          |
| value     | 存储IMSI的buf     |
| len       | 存储IMSI的buf长度 |
| **返回**  | **说明**          |
| OS\_EOK   | 成功              |
| 非OS\_EOK | 失败              |

##### mo_get_iccid

该函数用于获取ICCID，其函数原型如下：

```c
os_err_t mo_get_iccid(mo_object_t *self, char *value, os_size_t len);
```

| **参数**  | **说明**           |
| :-------- | :----------------- |
| self      | 模组对象           |
| value     | 存储ICCID的buf     |
| len       | 存储ICCID的buf长度 |
| **返回**  | **说明**           |
| OS\_EOK   | 成功               |
| 非OS\_EOK | 失败               |

##### mo_get_cfun

该函数用于获取射频模式，其函数原型如下：

```c
os_err_t mo_get_cfun(mo_object_t *self, os_uint8_t *fun_lvl);
```

| **参数**  | **说明**           |
| :-------- | :----------------- |
| self      | 模组对象           |
| fun_lvl   | 存储射频模式的指针 |
| **返回**  | **说明**           |
| OS\_EOK   | 成功               |
| 非OS\_EOK | 失败               |

##### mo_set_cfun

该函数用于设置射频模式，其函数原型如下：

```c
os_err_t mo_set_cfun(mo_object_t *self, os_uint8_t fun_lvl);
```

| **参数**  | **说明** |
| :-------- | :------- |
| self      | 模组对象 |
| fun_lvl   | 射频模式 |
| **返回**  | **说明** |
| OS\_EOK   | 成功     |
| 非OS\_EOK | 失败     |

*) fun_lvl的设置根据模组不同有所区别，需要具体查阅AT手册对应的值进行设置

##### mo_get_firmware_version

该函数用于获取模组的固件版本信息

```c
os_err_t mo_get_firmware_version(mo_object_t *self, mo_firmware_version_t *version);
```

| **参数**  | **说明**                     |
| :-------- | :--------------------------- |
| self      | 模组对象                     |
| version   | 存储固件版本号的结构体的指针 |
| **返回**  | **说明**                     |
| OS\_EOK   | 成功                         |
| 非OS\_EOK | 失败                         |

*) 该函数将动态申请用于存储固件版本信息的内存，调用该函数需调用mo_get_firmware_version_free函数释放内存。

##### mo_get_firmware_version_free

该函数用于释放获取的模组固件版本信息

```c
void mo_get_firmware_version_free(mo_firmware_version_t *version);
```

| **参数** | **说明**                     |
| :------- | :--------------------------- |
| version  | 存储固件版本号的结构体的指针 |

##### mo_get_eid

该函数用于获取SIM卡eID，其函数原型如下：

```c
os_err_t mo_get_eid(mo_object_t *self, char *eid, os_size_t len);
```

| **参数**  | **说明**      |
| :-------- | :------------ |
| self      | 模组对象      |
| eid       | 存储eID的指针 |
| len       | eID字符串长度 |
| **返回**  | **说明**      |
| OS\_EOK   | 成功          |
| 非OS\_EOK | 失败          |

##### mo_gm_time

该函数用于从模组获取本地分解时间，其函数原型如下：

```c
os_err_t mo_gm_time(mo_object_t *self, struct tm *l_tm);
```

| **参数**  | **说明**      |
| :-------- | :------------ |
| self      | 模组对象      |
| l_tm      | 存储分解时间结构体的指针 |
| **返回**  | **说明**      |
| OS\_EOK   | 成功          |
| 非OS\_EOK | 失败          |

##### mo_time

该函数用于获取本地UNIX时间，其函数原型如下：

```c
os_err_t mo_time(mo_object_t *self, time_t *timep);
```

| **参数**  | **说明**      |
| :-------- | :------------ |
| self      | 模组对象      |
| timep     | 存储UNIX时间的指针 |
| **返回**  | **说明**      |
| OS\_EOK   | 成功          |
| 非OS\_EOK | 失败          |

</details>

------

### **2.2 使用示例**

<details><summary>
<b>Samples</b>
</summary>

```c
#define IMEI_LEN    (15)
#define IMSI_LEN    (15)
#define ICCID_LEN   (20)
#define EID_LEN     (20)

os_err_t   result         = OS_ERROR;
os_uint8_t get_cfun_lvl   = 0;
os_uint8_t set_cfun_lvl   = 1;
time_t     time           = 0;
struct     tm l_tm;

char imei[IMEI_LEN + 1]         = {0};
char imsi[IMSI_LEN + 1]         = {0};
char iccid[ICCID_LEN + 1]       = {0};
char eid[EID_LEN + 1]           = {0};
mo_firmware_version_t version   = {0};


mo_object_t *test_module = mo_get_default();
OS_ASSERT(OS_NULL != test_module);

/* test AT & test connection */
result = mo_at_test(test_module);
OS_ASSERT(OS_EOK == result);

/* get IMEI */
result = mo_get_imei(test_module, imei, sizeof(imei));
OS_ASSERT(OS_EOK == result);
os_kprintf("module imei:%s\r\n", imei);

/* get IMSI */
result = mo_get_imsi(test_module, imsi, sizeof(imsi));
OS_ASSERT(OS_EOK == result);
os_kprintf("module imsi:%s\r\n", imsi);

/* get ICCID */
result = mo_get_iccid(test_module, iccid, sizeof(iccid));
OS_ASSERT(OS_EOK == result);
os_kprintf("module iccid:%s\r\n", iccid);

/* set function level */
result = mo_set_cfun(test_module, set_cfun_lvl);
OS_ASSERT(OS_EOK == result);

/* get function level */
result = mo_get_cfun(test_module, &get_cfun_lvl);
OS_ASSERT(OS_EOK == result);
os_kprintf("module cfun:%u\r\n", cfun_lvl);

/* get module firmware version */
os_err_t result = mo_get_firmware_version(test_module, &version);
OS_ASSERT(OS_EOK == result);

for (int i = 0; i < version.line_counts; i++)
{
    os_kprintf("%s\n", version.ver_info[i]);
}

mo_get_firmware_version_free(&version);

/* get eID */
result = mo_get_eid(test_module, eid, EID_LEN);
OS_ASSERT(OS_EOK == result);
os_kprintf("module eid:%s\r\n", eid);

/* get broken-down time */
result = mo_gm_time(test_module, &l_tm);
OS_ASSERT(OS_EOK == result);

/* get UNIX time */
result = mo_time(test_module, &time);
OS_ASSERT(OS_EOK == result);
```

</details>

------

## **三、网络服务接口**

网络服务接口提供模组网络服务相关基本信息及功能查询设置，部分功能在模组侧有依赖关系，具体见不同模组的AT手册。

<details open><summary>
<b>网络服务接口总览</b>
</summary>

| **接口**               | **说明**                           |
| :--------------------- | :--------------------------------- |
| mo_set_attach          | 网络附着或去附着                   |
| mo_get_attach          | 获取网络附着状态                   |
| mo_set_reg             | 设置网络注册参数                   |
| mo_get_reg             | 获取网络注册状态                   |
| mo_set_cgact           | 网络激活或去激活                   |
| mo_get_cgact           | 获取网络激活状态                   |
| mo_get_csq             | 获取信号强度                       |
| mo_get_radio           | 获取无线信息                       |
| mo_get_cell_info       | 获取cell信息                       |
| mo_set_psm             | 设置PSM选项                        |
| mo_get_psm             | 查询PSM信息                        |
| mo_set_edrx_cfg        | 配置edrx参数                       |
| mo_get_edrx_cfg        | 查询edrx配置                       |
| mo_get_edrx_dynamic    | 查询edrx生效值(读取动态 eDRX 参数) |
| mo_set_band            | 多频段模块设置搜网的频段           |
| mo_set_earfcn          | 锁频                               |
| mo_get_earfcn          | 查询earfcn(锁频)信息               |
| mo_clear_stored_earfcn | 清除存储的频点信息                 |
| mo_clear_plmn          | 清除plmn等驻网记录                 |

</details>

------

### **3.1 网络服务接口说明**

<details open><summary>
<b>APIs</b>
</summary>

##### mo_set_attach

该函数用于附着或去附着，其函数原型如下：

```c
os_err_t mo_set_attach(mo_object_t *self, os_uint8_t attach_stat);
```

| **参数**    | **说明**       |
| :---------- | :------------- |
| self        | 模组对象       |
| attach_stat | 欲设置附着状态 |
| **返回**    | **说明**       |
| OS\_EOK     | 成功           |
| 非OS\_EOK   | 失败           |

##### mo_get_attach

该函数用于获取附着状态，其函数原型如下：

```c
os_err_t mo_get_attach(mo_object_t *self, os_uint8_t *attach_stat);
```

| **参数**    | **说明**          |
| :---------- | :---------------- |
| self        | 模组对象          |
| attach_stat | 存储附着状态的buf |
| **返回**    | **说明**          |
| OS\_EOK     | 成功              |
| 非OS\_EOK   | 失败              |

##### mo_set_reg

该函数用于设置注册参数，其函数原型如下：

```c
os_err_t mo_set_reg(mo_object_t *self, os_uint8_t reg_n);
```

| **参数**  | **说明** |
| :-------- | :------- |
| self      | 模组对象 |
| reg_n     | 注册参数 |
| **返回**  | **说明** |
| OS\_EOK   | 成功     |
| 非OS\_EOK | 失败     |

##### mo_get_reg

该函数用于获取注册状态，其函数原型如下：

```c
os_err_t mo_get_reg(mo_object_t *self, eps_reg_info_t *info);
```

| **参数**  | **说明**          |
| :-------- | :---------------- |
| self      | 模组对象          |
| info      | 存储注册状态的结构体指针 |
| **返回**  | **说明**          |
| OS\_EOK   | 成功              |
| 非OS\_EOK | 失败              |

##### mo_set_cgact

该函数用于激活或去激活，其函数原型如下：

```c
os_err_t mo_set_cgact(mo_object_t *self, os_uint8_t cid, os_uint8_t act_n);
```

| **参数**  | **说明**       |
| :-------- | :------------- |
| self      | 模组对象       |
| cid       | CID参数        |
| act_n     | 激活参数，0或1 |
| **返回**  | **说明**       |
| OS\_EOK   | 成功           |
| 非OS\_EOK | 失败           |

##### mo_get_cgact

该函数用于获取激活状态，其函数原型如下：

```c
os_err_t mo_get_cgact(mo_object_t *self, os_uint8_t *cid, os_uint8_t *act_stat);
```

| **参数**  | **说明**          |
| :-------- | :---------------- |
| self      | 模组对象          |
| cid       | 存储CID参数的buf  |
| act_stat  | 存储激活参数的buf |
| **返回**  | **说明**          |
| OS\_EOK   | 成功              |
| 非OS\_EOK | 失败              |

##### mo_get_csq

该函数用于获取信号强度，其函数原型如下：

```c
os_err_t mo_get_csq(mo_object_t *self, os_uint8_t *rssi, os_uint8_t *ber);
```

| **参数**  | **说明**      |
| :-------- | :------------ |
| self      | 模组对象      |
| rssi      | 存储RSSI的buf |
| act_stat  | 存储BER的buf  |
| **返回**  | **说明**      |
| OS\_EOK   | 成功          |
| 非OS\_EOK | 失败          |

##### mo_get_radio

该函数用于获取无线信息，其函数原型如下：

```c
os_err_t mo_get_radio(mo_object_t *self, radio_info_t *radio_info);
```

| **参数**   | **说明**          |
| :--------- | :---------------- |
| self       | 模组对象          |
| radio_info | 存储无线信息的buf |
| **返回**   | **说明**          |
| OS\_EOK    | 成功              |
| 非OS\_EOK  | 失败              |

##### mo_get_cell_info

该函数用于获取cell信息，其函数原型如下：

```c
os_err_t mo_get_cell_info(mo_object_t *self, onepos_cell_info_t* onepos_cell_info);
```

| **参数**         | **说明**          |
| :--------------- | :---------------- |
| self             | 模组对象          |
| onepos_cell_info | 存储cell信息的buf |
| **返回**         | **说明**          |
| OS\_EOK          | 成功              |
| 非OS\_EOK        | 失败              |

##### mo_set_psm

该函数用于设置PSM选项，其函数原型如下：

```c
os_err_t mo_set_psm(mo_object_t *self, mo_psm_info_t info);
```

| **参数**  | **说明**         |
| :-------- | :--------------- |
| self      | 模组对象         |
| info      | 设置PSM选项的buf |
| **返回**  | **说明**         |
| OS\_EOK   | 成功             |
| 非OS\_EOK | 失败             |

##### mo_get_psm

该函数用于获取PSM选项，其函数原型如下：

```c
os_err_t mo_get_psm(mo_object_t *self, mo_psm_info_t *info);
```

| **参数**  | **说明**         |
| :-------- | :--------------- |
| self      | 模组对象         |
| info      | 存储PSM选项的buf |
| **返回**  | **说明**         |
| OS\_EOK   | 成功             |
| 非OS\_EOK | 失败             |

##### mo_set_edrx_cfg

该函数用于配置edrx参数信息，其函数原型如下：

```c
os_err_t mo_set_edrx_cfg(mo_object_t *self, mo_edrx_cfg_t cfg);
```

| **参数**  | **说明**          |
| :-------- | :---------------- |
| self      | 模组对象          |
| cfg       | 配置edrx参数的buf |
| **返回**  | **说明**          |
| OS\_EOK   | 成功              |
| 非OS\_EOK | 失败              |

> *) 具体数值依照对应AT手册。另``mo_edrx_cfg_t``为复用结构体，此接口设置eDRX无需填入``nw_edrx_value<NW-provided_eDRX_value>``和``paging_time_window<Paging_time_window>``字段。

##### mo_get_edrx_cfg

该函数用于查询edrx配置信息，其函数原型如下：

```c
os_err_t mo_get_edrx_cfg(mo_object_t *self, mo_edrx_t *edrx_local);
```

| **参数**  | **说明**          |
| :-------- | :---------------- |
| self      | 模组对象          |
| info      | 存储edrx配置的buf |
| **返回**  | **说明**          |
| OS\_EOK   | 成功              |
| 非OS\_EOK | 失败              |

##### mo_get_edrx_dynamic

该函数用于获取查询edrx生效值(读取动态 eDRX 参数)信息，其函数原型如下：

```c
os_err_t mo_get_edrx_dynamic(mo_object_t *self, mo_edrx_t *edrx_dynamic);
```

| **参数**  | **说明**            |
| :-------- | :------------------ |
| self      | 模组对象            |
| param     | 存储edrx生效值的buf |
| **返回**  | **说明**            |
| OS\_EOK   | 成功                |
| 非OS\_EOK | 失败                |

##### mo_set_band

该函数用于配置多频段模块设置搜网的频段信息，其函数原型如下：

```c
os_err_t mo_set_band(mo_object_t *self, char band_list[], os_uint8_t num);
```

| **参数**  | **说明**            |
| :-------- | :------------------ |
| self      | 模组对象            |
| band_list | 存储频段信息的buf   |
| num       | 存储band_list的长度 |
| **返回**  | **说明**            |
| OS\_EOK   | 成功                |
| 非OS\_EOK | 失败                |

##### mo_set_earfcn

该函数用于设置锁频选项，其函数原型如下：

```c
os_err_t mo_set_earfcn(mo_object_t *self, mo_earfcn_t earfcn);
```

| **参数**  | **说明**          |
| :-------- | :---------------- |
| self      | 模组对象          |
| earfcn    | 设置锁频选项的buf |
| **返回**  | **说明**          |
| OS\_EOK   | 成功              |
| 非OS\_EOK | 失败              |

> *) 备注:具体设置详见各模组AT手册所定义。移远BC95/BC28模组`earfcn_offset`无需设置。

##### mo_get_earfcn

该函数用于查询earfcn(锁频)信息 ，其函数原型如下：

```c
os_err_t mo_get_earfcn(mo_object_t *self, mo_earfcn_t *earfcn);
```

| **参数**  | **说明**                  |
| :-------- | :------------------------ |
| self      | 模组对象                  |
| earfcn    | 存储锁频配置相关信息的buf |
| **返回**  | **说明**                  |
| OS\_EOK   | 成功                      |
| 非OS\_EOK | 失败                      |

##### mo_clear_stored_earfcn

该函数用于清除存储的频点信息，其函数原型如下：

```c
os_err_t mo_clear_stored_earfcn(mo_object_t *self);
```

| **参数**  | **说明** |
| :-------- | :------- |
| self      | 模组对象 |
| **返回**  | **说明** |
| OS\_EOK   | 成功     |
| 非OS\_EOK | 失败     |

##### mo_clear_plmn

该函数用于清除plmn等信息，其函数原型如下：

```c
os_err_t mo_clear_plmn(mo_object_t *self);
```

| **参数**  | **说明** |
| :-------- | :------- |
| self      | 模组对象 |
| **返回**  | **说明** |
| OS\_EOK   | 成功     |
| 非OS\_EOK | 失败     |

</details>

------

### **3.2 使用示例**

<details><summary>
<b>Samples</b>
</summary>

```c
#include <mo_api.h>

#define INIT_DFT       (0)
#define INIT_BER_DFT   (99)
#define TEST_CID       (1)

/* for example */
#typedef enum test_attach_stat
{
    DETACHED = 0,
    ATTACHED,
    ATTACH_RESERVED,
} test_attach_stat_t;

/* for example */
#typedef enum test_reg_stat
{
    DISABLE_REG_URC = 0,
    ENABLE_REG_URC,
    ENABLE_REG_LO_URC,
    ENABLE_REG_LO_EMM_URC,
    PSM_ENABLE_REG_LO_URC,
    PSM_ENABLE_REG_LO_EMM_URC,
} test_reg_stat_t;

/* for example */
#typedef enum test_act_stat
{
    DEACTIVATED = 0,
    ACTIVATED,
    ACTIVATE_RESERVED,

} test_act_stat_t;

os_err_t   result          = OS_ERROR;
os_uint8_t attach_stat_set = TEST_ATTACHED;
os_uint8_t attach_stat_get = INIT_DFT;
os_uint8_t reg_n           = ENABLE_REG_URC;
os_uint8_t cid_set         = TEST_CID;
os_uint8_t cid_get         = INIT_DFT;
os_uint8_t act_stat_set    = DEACTIVATED;
os_uint8_t act_stat_get    = INIT_DFT;
os_uint8_t rssi            = INIT_DFT;
os_uint8_t ber             = INIT_BER_DFT;

radio_info_t radio_info              = {0};
char ip_addr[IPADDR_MAX_STR_LEN + 1] = {0};

mo_psm_info_t psm_enable  = {MO_PSM_ENABLE, "", "", "00100011", "00100010"};
mo_psm_info_t psm_disable = {MO_PSM_DISABLE, "", "", "00100011", "00100010"};

eps_reg_info_t reg_info;
memset(&reg_info, 0, sizeof(eps_reg_info_t));
onepos_cell_info_t cell_info;
memset(&cell_info, 0, sizeof(onepos_cell_info_t));
mo_psm_info_t psm_info_get;
memset(&psm_info_get, 0, sizeof(mo_psm_info_t));

mo_object_t *test_module = mo_get_default();
OS_ASSERT(OS_NULL != test_module);

/* set module attach state */
mo_set_attach(est_module, attach_stat_set);
OS_ASSERT(OS_EOK == result);

/* get module attach state */
result = mo_get_attach(test_module, &attach_stat_get);
OS_ASSERT(OS_EOK == result);
os_kprintf("module attach state:%u\r\n", attach_stat_get);


/* set the presentation of an network registration urc data */
result = mo_set_reg(test_module, reg_n);
OS_ASSERT(OS_EOK == result);

/*
 * get the presentation of an network registration urc data
 * and the network registration status
 */
result = mo_get_reg(test_module, &reg_info);
OS_ASSERT(OS_EOK == result);
os_kprintf("module register n:%u"\r\n,     reg_info.reg_n);
os_kprintf("module register state:%u\r\n", reg_info.reg_stat);

/* set activate or deactivate PDP Context */
result = mo_set_cgact(test_module, cid_set, act_stat_set);
OS_ASSERT(OS_EOK == result);

/* get the state of PDP context activation */
result = mo_get_cgact(test_module, &cid_get, &act_stat_get);
OS_ASSERT(OS_EOK == result);
os_kprintf("module PDP context cid:%u,act stat:%u\r\n", cid_get, act_stat_get);

/* get the csq info */
result = result = mo_get_csq(test_module, &rssi, &ber);
OS_ASSERT(OS_EOK == result);
os_kprintf("module csq rssi:%u\r\n", rssi);
os_kprintf("module csq ber:%u\r\n", ber);

/* get module radio info */
result = mo_get_radio(test_module, &radio_info);
OS_ASSERT(OS_EOK == result);
os_kprintf("module cell id:%s\r\n", radio_info.cell_id);
os_kprintf("module ecl:%d\r\n",     radio_info.ecl);
os_kprintf("module snr:%d\r\n",     radio_info.snr);
os_kprintf("module earfcn:%d\r\n",  radio_info.earfcn);
os_kprintf("module rsrq:%d\r\n",    radio_info.rsrq);

/* get cell info */
result = mo_get_cell_info(test_module, &cell_info);
OS_ASSERT(OS_EOK == result);
os_kprintf("module cell_info cell_num:%d\r\n", cell_info.cell_num);
os_kprintf("module cell_info net_type:%d\r\n", cell_info.net_type);
os_kprintf("module cell_info mnc:%d\r\n",      cell_info.cell_info->mnc);
os_kprintf("module cell_info mcc:%d\r\n",      cell_info.cell_info->mcc);
os_kprintf("module cell_info lac:%d\r\n",      cell_info.cell_info->lac);
os_kprintf("module cell_info cid:%d\r\n",      cell_info.cell_info->cid);
os_kprintf("module cell_info ss:%d\r\n",       cell_info.cell_info->ss);

/* set PSM(power saving mode) enable & other configuration */
result = mo_set_psm(test_module, psm_enable);
OS_ASSERT(OS_EOK == result);

/* get get module PSM(power saving mode) info */
result = mo_get_psm(test_module, &psm_info_get);
OS_ASSERT(OS_EOK == result);
os_kprintf("module psm_mode:%d\r\n",         psm_info_get.psm_mode);
os_kprintf("module periodic_rau:%s\r\n",     psm_info_get.periodic_rau);
os_kprintf("module gprs_ready_timer:%s\r\n", psm_info_get.gprs_ready_timer);
os_kprintf("module periodic_tau:%s\r\n",     psm_info_get.periodic_tau);
os_kprintf("module active_time:%s\r\n",      psm_info_get.active_time);
```

</details>

------

## **四、套接字接口**

### **4.1 套接字接口说明**

MoLink提供套接字接口，区分于普通套接字接口，以``mo_``作标志，使用方式与普通套接字接口基本无异，区别在于某些接口需要传入模组实例。
> 详见
[``components\net\molink\api\include\mo_socket.h``](../api/include/mo_socket.h)

另``OneOS socket组件``为用户提供了一套兼容BSD的标准接口，用户可在[``自动创建``](#121-自动创建)时选择使能BSD套接字服务，即可使用。


------

### **4.2 常见问题**

Q): 为何发送/接收较长数据时返回超时或错误，且很难返回成功？

A): 对于NB类型模组，因网络因素受限，收发较大长度的数据常会有严重丢包重传等现象发生，不建议大量数据收发。

------

## **五、MQTT客户端接口**

MQTT客户端提供接口来连接到MQTT代理来发布消息、订阅主题和接收发布的消息。

<details open><summary>
<b>MQTT客户端接口总览</b>
</summary>

| **接口**                 | **说明**                         |
| :----------------------- | :------------------------------- |
| mo_mqttc_create          | 创建MQTT客户端                   |
| mo_mqttc_destroy         | 销毁MQTT客户端                   |
| mo_mqttc_connect         | 连接至MQTT代理                   |
| mo_mqttc_publish         | 发布一条消息                     |
| mo_mqttc_subscribe       | 订阅一个主题                     |
| mo_mqttc_unsubscribe     | 取消订阅的主题                   |
| mo_mqttc_set_msg_handler | 设置或移除指定主题的处理函数     |
| mo_mqttc_disconnect      | 断开与MQTT代理的连接             |
| mo_mqttc_isconnect       | 检查是否连接至MQTT代理           |
| mo_mqttc_yield           | 接收并处理MQTT消息               |
| mo_mqttc_start_task      | 启动一个任务来接收并处理MQTT消息 |

</details>

------

### **5.1 MQTT客户端接口说明**

<details open><summary>
<b>APIs</b>
</summary>

##### mo_mqttc_create

该函数用于创建一个MQTT客户端实例，其函数原型如下：

```c
mo_mqttc_t *mo_mqttc_create(mo_object_t *module, mqttc_create_opts_t *create_opts)
```

| **参数**    | **说明**           |
| :---------- | :----------------- |
| module      | 模组对象           |
| create_opts | MQTT客户端创建选项 |
| **返回**    | **说明**           |
| 非OS\_NULL  | 成功               |
| OS\_NULL    | 失败               |

MQTT客户端创建选项说明如下：

```c
typedef struct mqttc_create_opts
{
    mqttc_string_t address;         /* The MQTT server address */
    os_uint16_t    port;            /* The MQTT server port */
    os_uint32_t    command_timeout; /* The MQTT client ACK timeout, unit ms*/
    os_size_t      max_msgs;        /* The max number of messages.*/
} mqttc_create_opts_t;
```

| **重要成员**    | **说明**                                                        |
| :-------------- | :-------------------------------------------------------------- |
| address         | 服务器地址字符串                                                |
| port            | 服务器端口号                                                    |
| command_timeout | MQTT客户端ACK超时时间，单位为ms，若为0则使用模组提供的默认值    |
| max_msgs        | MQTT客户端一次接收的最大的消息数量，若为0则使用系统提供的默认值 |

##### mo_mqttc_connect

该函数用于连接至MQTT代理服务器，其函数原型如下：

```c
os_err_t mo_mqttc_connect(mo_mqttc_t *client, mqttc_conn_opts_t *connect_opts)
```

| **参数**     | **说明**           |
| :----------- | :----------------- |
| client       | MQTT客户端实例     |
| connect_opts | MQTT客户端连接选项 |
| **返回**     | **说明**           |
| OS\_EOK      | 成功               |
| 非OS\_EOK    | 失败               |

MQTT客户端连接选项说明如下：

```c
typedef struct mqttc_conn_opts
{
    mqttc_string_t client_id;    /* The MQTT client identifier string */

    os_uint8_t  mqtt_version;    /* The Version of MQTT to be used.  3 = 3.1 4 = 3.1.1 */
    os_uint32_t keep_alive;      /* The keep alive time, unit second */
    os_uint8_t  clean_session;   /* The flag of clean_session option */
    os_uint8_t  will_flag;       /* The flag of will option */

    mqttc_will_opts_t will_opts; /* The options of will */

    mqttc_string_t username;
    mqttc_string_t password;
} mqttc_conn_opts_t;
```

| **重要成员**  | **说明**                                                                  |
| :------------ | :------------------------------------------------------------------------ |
| client_id     | MQTT客户端标识                                                            |
| mqtt_version  | MQTT协议的版本，支持的版本号取由模组决定                                  |
| keep_alive    | MQTT客户端与服务器心跳保活时间，单位为秒，若设置为0则使用模组提供的默认值 |
| clean_session | 是否清除session信息                                                       |
| will_flag     | 是否使用遗言                                                              |
| will_opts     | 遗言选项                                                                  |
| username      | 用户名                                                                    |
| password      | 用户密钥                                                                  |

MQTT客户端遗愿选项说明如下：

```c
typedef struct mqttc_will_opts
{
    mqttc_string_t topic_name; /* The LWT topic to which the LWT message will be published */
    mqttc_string_t message;    /* The LWT payload */
    mqttc_qos_t    qos;        /* The quality of service setting for the LWT message */
    os_uint8_t     retained;   /* The retained flag for the LWT message */
} mqttc_will_opts_t;
```

| **重要成员** | **说明**       |
| :----------- | :------------- |
| topic_name   | 遗愿的主题     |
| message      | 遗愿的消息内容 |
| qos          | 遗愿的服务等级 |
| retained     | 遗愿是否保留   |

##### mo_mqttc_publish

该函数用于向指定的主题发布一条消息，其函数原型如下：

```c
os_err_t mo_mqttc_publish(mo_mqttc_t *client, const char *topic_filter, mqttc_msg_t *msg);
```

| **参数**     | **说明**           |
| :----------- | :----------------- |
| client       | MQTT客户端实例     |
| topic_filter | 需要发布的消息主题 |
| msg          | 需要发布的消息     |
| **返回**     | **说明**           |
| OS\_EOK      | 成功               |
| 非OS\_EOK    | 失败               |

MQTT消息结构体的说明如下:

```c
typedef struct mqttc_msg
{
    mqttc_qos_t qos;       /* The quality of service setting for the message */

    os_uint8_t  retained;  /* The retained flag for the message */
    os_uint8_t  dup;       /* The dup flag for the message*/

    void     *payload;     /* The data of the message */
    os_size_t payload_len; /* The length of data */
} mqttc_msg_t;

```

| **重要成员** | **说明**     |
| :----------- | :----------- |
| qos          | 服务质量     |
| retained     | 是否保留     |
| dup          | 重发标识     |
| payload      | 消息数据     |
| payloadlen   | 消息数据长度 |

##### mo_mqttc_subscribe

该函数用于订阅指定的主题并设置消息处理函数，其函数原型如下：

```c
os_err_t mo_mqttc_subscribe(mo_mqttc_t *client,
                            const char *topic_filter,
                            mqttc_qos_t qos,
                            mqttc_msg_handler_t handler);
```

| **参数**     | **说明**               |
| :----------- | :--------------------- |
| client       | MQTT客户端实例         |
| topic_filter | 需要订阅的消息主题     |
| qos          | 订阅消息的服务质量     |
| handler      | 订阅主题消息的处理函数 |
| **返回**     | **说明**               |
| OS\_EOK      | 成功                   |
| 非OS\_EOK    | 失败                   |

订阅主题消息的处理函数的说明如下:

```c
typedef void (*mqttc_msg_handler_t)(mqttc_msg_data_t*);
```

函数中的消息数据结构体的说明如下:

```c
typedef struct mqttc_msg_data
{
    mqttc_msg_t    message;
    mqttc_string_t topic_name;
} mqttc_msg_data_t;
```

| **重要成员** | **说明**       |
| :----------- | :------------- |
| message      | MQTT消息结构体 |
| topic_name   | 消息主题       |

##### mo_mqttc_unsubscribe

该函数取消指定的主题的订阅，其函数原型如下：

```c
os_err_t mo_mqttc_unsubscribe(mo_mqttc_t *client, const char *topic_filter)
```

| **参数**     | **说明**               |
| :----------- | :--------------------- |
| client       | MQTT客户端实例         |
| topic_filter | 需要取消订阅的消息主题 |
| **返回**     | **说明**               |
| OS\_EOK      | 成功                   |
| 非OS\_EOK    | 失败                   |

##### mo_mqttc_set_msg_handler

该函数用于设置或移除指定主题的处理函数，其函数原型如下：

```c
os_err_t mo_mqttc_set_msg_handler(mo_mqttc_t *client,
                                  const char *topic_filter,
                                  mqttc_msg_handler_t handler);
```

| **参数**     | **说明**                                              |
| :----------- | :---------------------------------------------------- |
| client       | MQTT客户端实例                                        |
| topic_filter | 需要设置的消息主题                                    |
| handler      | 消息处理函数的指针，如果为OS_NULL则移除原有的处理函数 |
| **返回**     | **说明**                                              |
| OS\_EOK      | 成功                                                  |
| 非OS\_EOK    | 失败                                                  |

##### mo_mqttc_disconnect

该函数用于断开与MQTT代理服务器的连接，其函数原型如下：

```c
os_err_t mo_mqttc_disconnect(mo_mqttc_t *client);
```

| **参数**  | **说明**       |
| :-------- | :------------- |
| client    | MQTT客户端实例 |
| **返回**  | **说明**       |
| OS\_EOK   | 成功           |
| 非OS\_EOK | 失败           |

##### mo_mqttc_isconnect

该函数检测是否连接至MQTT代理服务器，其函数原型如下：

```c
os_bool_t mo_mqttc_isconnect(mo_mqttc_t *client);
```

| **参数**    | **说明**       |
| :---------- | :------------- |
| client      | MQTT客户端实例 |
| **返回**    | **说明**       |
| OS\_TURE    | 已连接         |
| 非OS\_FALSE | 未连接         |

##### mo_mqttc_destroy

该函数用于销毁一个MQTT客户端实例，该函数原型如下：

```c
os_err_t mo_mqttc_destroy(mo_mqttc_t *client)
```

| **参数**  | **说明**       |
| :-------- | :------------- |
| client    | MQTT客户端实例 |
| **返回**  | **说明**       |
| OS\_EOK   | 成功           |
| 非OS\_EOK | 失败           |

> *) 销毁客户端前请先调用mo_mqttc_disconnect断开与服务器的连接。

##### mo_mqttc_yield

定时调用该函数来处理接收到的MQTT消息，该函数原型如下：

```c
os_err_t mo_mqttc_yield(mo_mqttc_t *client, os_uint32_t timeout_ms)
```

| **参数**   | **说明**                         |
| :--------- | :------------------------------- |
| client     | MQTT客户端实例                   |
| timeout_ms | 等待处理MQTT消息的时间，单位毫秒 |
| **返回**   | **说明**                         |
| OS\_EOK    | 成功                             |
| 非OS\_EOK  | 失败                             |

##### mo_mqttc_start_task

该函数用于启动一个线程来处理接收到的MQTT消息，该函数原型如下：

```c
os_err_t mo_mqttc_start_task(mo_mqttc_t *client);
```

| **参数**  | **说明**       |
| :-------- | :------------- |
| client    | MQTT客户端实例 |
| **返回**  | **说明**       |
| OS\_EOK   | 成功           |
| 非OS\_EOK | 失败           |

> *) 调用该函数后，就不应该再调用mo_mqttc_yield函数。

</details>

------

### **5.2 使用示例**

<details><summary>
<b>Samples</b>
</summary>

```c

static void test_mqttc_handler(mqttc_msg_data_t *data)
{
    os_kprintf("Message arrived on topic %.*s: %.*s\n",
               data->topic_name.len,
               data->topic_name.data,
               data->message.payload_len,
               data->message.payload);
}

static void test_mqttc_start_task(void)
{
    mqttc_create_opts_t create_opts = {
        .address = 
        {
            .data = TEST_HOST, 
            .len = strlen(TEST_HOST)
        }, 
        .port = TEST_PORT
    };

    mo_mqttc_t *mqttc = mo_mqttc_create(test_module, &create_opts);

    mqttc_conn_opts_t conn_opts = {
        .client_id = {
            .data = TEST_CLIENT_ID,
            .len  = strlen(TEST_CLIENT_ID),
        },
        .mqtt_version  = 4,
        .keep_alive    = 60,
        .clean_session = 1,
        .will_flag     = 0,
    };

    os_err_t result = mo_mqttc_connect(mqttc, &conn_opts);

    result = mo_mqttc_start_task(mqttc);

    result = mo_mqttc_subscribe(mqttc, TEST_TOPIC, MQTTC_QOS_1, test_mqttc_handler);

    for (int i = 0; i < 5; i++)
    {
        char payload[64] = {0};
        sprintf(payload, "message number %d", i);

        mqttc_msg_t msg = {
            .qos = MQTTC_QOS_1, 
            .retained = 0, 
            .payload = payload, 
            .payload_len = strlen(payload)
        };

        result = mo_mqttc_publish(mqttc, TEST_TOPIC, &msg);
    }

    mo_mqttc_disconnect(mqttc);

    mo_mqttc_destroy(mqttc);
}

```

</details>

------

| [回到顶部](#molink-api) | [📖目录](#目录) | [🌍中文](#molink-api) | [🌍ENGLISH](#./molink_api_en.md) | [<img src="./images/api_oneos_logo.png" width="42.5" height="30" align="bottom" />](https://os.iot.10086.cn/)
