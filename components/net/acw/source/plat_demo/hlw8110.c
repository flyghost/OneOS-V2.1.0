/*
 * @Author: lizezhi
 * @Date: 2021-03-03 14:01:14
 * @LastEditTime: 2021-03-03 17:57:17
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 */

#include <os_memory.h>
#include <os_stddef.h>
#include <string.h>

#include "hlw8110.h"

#if 0
#define SS_LOG(ml_lvl_, ...) os_kprintf(__VA_ARGS__)
#else
#define SS_LOG(ml_lvl_, ...)
#endif
static gc_uart_t *gc_uart;
static os_uint32_t  mb_pool[4];
static os_bool_t gs_hlw8110_init;
static hlw8110_actual_factor_t hlw8110_actual_factor;
static hlw8110_actual_result_t hlw8110_actual_result;
static os_mailbox_t rx_mb;

static os_err_t gc_serial_tx_done(os_device_t *uart, struct os_device_cb_info *info)
{
    return 0;
}

/* receive package to buffer*/
static os_err_t gc_serial_rx_done(os_device_t *uart, struct os_device_cb_info *info)
{
    SS_LOG(SS_LOG_LEVEL_TRACE, "************************rx done************************\r\n");
    os_mb_send(&rx_mb, (os_uint32_t)info->size, OS_IPC_WAITING_NO);
    return 0;
}

/**
* @param[in] 
*
*/
void* gc_serial_open(void)
{
     gc_uart_t *u;
     int           ret;
    os_device_t  *uart;

     u = (gc_uart_t *)os_malloc(sizeof(*u));
    if (u == NULL)
    {
        SS_LOG(SS_LOG_LEVEL_ERR,"alloc uart_socket failed!\n");
        return (NULL);
    }
    memset(u, 0x0, sizeof(*u));
    if (OS_EOK != os_mb_init(&rx_mb, "gc_rx_mb", mb_pool, sizeof(mb_pool) / sizeof(mb_pool[0]), OS_IPC_FLAG_FIFO))
        SS_LOG(SS_LOG_LEVEL_ERR,"uart init mb failed! \n");

    uart = os_device_find(HLW8110_UART_PORT);
    OS_ASSERT(uart);

    /*cb_info*/
    struct os_device_cb_info cb_info;
    cb_info.type = OS_DEVICE_CB_TYPE_TX;
    cb_info.cb   = NULL;                                                //no this callback so far
    os_device_control(uart, IOC_SET_CB, &cb_info);

    cb_info.type = OS_DEVICE_CB_TYPE_RX;
    cb_info.cb   = gc_serial_rx_done;
    os_device_control(uart, IOC_SET_CB, &cb_info);

    /* Open serial device with rx nopoll flag */
    os_device_open(uart, OS_SERIAL_FLAG_RX_NOPOLL | OS_SERIAL_FLAG_TX_NOPOLL | OS_DEVICE_OFLAG_RDWR);
    if (OS_EOK != ret) {
        SS_LOG(SS_LOG_LEVEL_ERR, "serial open fail %d\r\n", ret);
        os_free(u);
        return NULL;
    }
    /*device init*/
    struct serial_configure conf = OS_SERIAL_CONFIG_DEFAULT;
    conf.baud_rate = HLW8110_UART_BAUD;
    conf.parity = HLW8110_UART_PARITY; 
    ret = os_device_control(uart, OS_DEVICE_CTRL_CONFIG, &conf);
    if (ret != OS_EOK)
    {
        SS_LOG(SS_LOG_LEVEL_ERR, "serial set baud fail %d\r\n", ret);    
        return NULL;
    }

    u->dev = uart;
    if (u->dev == NULL)
    {
        SS_LOG(SS_LOG_LEVEL_ERR, "serial dev not exist\r\n");
        os_free(u);
        return (NULL);
    }
    return u;
}

int gc_serial_close(void * dev)
{
    gc_uart_t *u;

    if (u == NULL)
    {
        return (OS_ERROR);;
    }

    os_device_close(u->dev);
    if(u != NULL)
        os_free(u);

    return OS_EOK;
}
int gc_serial_read(void * pbuf, os_uint16_t len)
{    
    int ret = 0;
    
    ret = os_device_read(gc_uart->dev, 0, (os_uint8_t *)pbuf, len);

     return ret;
}
int gc_serial_write(void *pbuf, os_uint16_t len)
{
    int ret = 0;

    ret = os_device_write(gc_uart->dev, 0, (os_uint8_t *)pbuf, len);

    return ret;
}

/***
 * 
 * @brief  calculate checksum 
 * @param buf the data. 
 * @param len need to calculate  
 * @returns checksum result 
 * */

os_uint8_t hlw8110_checksum(void *buf, os_uint16_t len)
{
    os_uint32_t ret = 0;

    os_uint8_t *pbuf;

    pbuf= os_malloc(len);

    memcpy(pbuf,buf,len);

    for(os_uint8_t i=0;i<len;i++) {
        ret += pbuf[i]; 
    }
    ret = (~ret);
    ret = (ret & 0xff);

    os_free(pbuf);
    return ret;
}

/**
 * @brief formula: (rms_i * rms_i_c)/(K1* 2^23）(ma)
 * @input rms_i: read from register
 * @input rms_i_c: factor from register
 * @return actual RMS_i (unit mA)
*/
os_uint32_t hlw8110_rms_i_calculate(os_uint32_t rms_i, os_uint16_t rms_i_c)
{
    os_uint32_t ret = 0;
    float a = 0;
    SS_LOG(SS_LOG_LEVEL_TRACE,"read register %d factor %d\r\n",rms_i,rms_i_c);
    if ((rms_i & 0x800000) == 0x800000) {
        ret = 0;
    }
    else {
        a = (float)rms_i;
        a = a * rms_i_c;
        a  = (a/0x800000);
        a = a/HLW8110_I_CHANNEL_SENSOR_K1;
        a = a * D_CAL_A_I;
        ret = a;
    }
    
    return ret;
}

/**
 * @brief formula ((rms_u * rms_u_c * D_CAL_U)/(K2 * pow_2(22))) (10mv);
 * @param[in] rms_u: read from register
 * @param[in] rms_u_c: factor from register
 * @return actual RMS_u (unit 100 mV)
*/
os_uint32_t hlw8110_rms_u_calculate(os_uint32_t rms_u, os_uint16_t rms_u_c)
{
    SS_LOG(SS_LOG_LEVEL_TRACE,"read register %d factor %d\r\n",rms_u,rms_u_c);
    os_uint32_t ret = 0;
    float a = 0;
    if ((rms_u & 0x800000) == 0x800000) {
        ret = 0;
    }
    else {
        a = (float)rms_u;
        a = a * rms_u_c;
        a  = (a/0x400000);
        a = a/HLW8110_U_CHANNEL_SENSOR_K2;
        a = a * D_CAL_U;
        a = a/10;
        ret = a;
    }
    
    return ret;
}

/**
 * @brief formula: ((power * power_c)/(K1 * K2 * (2^31)));
 * @param[in] power: read from register
 * @param[in] power_c: factor from register
 * @return power (unit 0.1w)
*/
os_uint32_t hlw8110_power_calculate(os_uint32_t power, os_uint16_t power_c)
{
    SS_LOG(SS_LOG_LEVEL_TRACE,"read register %d factor %d\r\n",power,power_c);
    os_uint32_t ret = 0;
    float a = 0;
    float b = 0;
    /* positive and negtive */
    if (power > 0x80000000) {
     b = ~power;
     a = (float)b;
   }
   else {
     a =  (float)power;
   }
    a = a*power_c;
    a = (a/0x80000000);
    a = a/HLW8110_I_CHANNEL_SENSOR_K1;
    a = a/HLW8110_U_CHANNEL_SENSOR_K2;
    a = a * D_CAL_A_P;
    ret = a*10;

    return ret;
}

/**
 * @brief formula: (((energy * energy_c) * HFCONST)/(K2 * K1 * (2^29) * 4096));
 * HFCONST: default 0x1000
 * @param[in] energy: read from register
 * @param[in] energy_c: factor from register
 * @return energy (unit 0.1w*h)
*/
os_uint32_t hlw8110_engergy_calculate(os_uint32_t energy, os_uint16_t energy_c)
{
    SS_LOG(SS_LOG_LEVEL_TRACE,"read register %d factor %d\r\n",energy,energy_c);
    os_uint32_t ret = 0;
    float a = 0;
    a =  (float)energy;
    a = a*energy_c;
    a = (a/0x20000000);   
    a = a/HLW8110_I_CHANNEL_SENSOR_K1;
    a = a/HLW8110_U_CHANNEL_SENSOR_K2;
    a = a * D_CAL_A_E;                          /*unit 1KWH*/
    ret = a*10000;
    
    return ret;
}

void hlw8110_write_reg(os_uint8_t reg, os_uint32_t len, os_uint32_t data)
{
    os_uint8_t pbuf[10] = {0};
    os_uint8_t pbuf_len = 3 + len;

    pbuf[0] = SMART_SOCKET_UART_HEADER;
    pbuf[1] = HLW8110_W_REG(reg);
    for (os_uint8_t i = 0; i< len; i++) {
        pbuf[2 + i] = (data >> ((len - 1 - i) * 8) & 0xff);
    }

    pbuf[pbuf_len - 1] = hlw8110_checksum(&pbuf[0],pbuf_len);
    gc_serial_write(pbuf,pbuf_len);
}


void hlw8110_write_reg_enable(void)
{
    hlw8110_write_reg(HLW8110_SPECIAL_REG,1,HLW8110_WRITE_ENABLE);
}

void hlw8110_set_channel_a(void)
{
    hlw8110_write_reg(HLW8110_SPECIAL_REG,1,HLW8110_SELECT_CHANNEL_A);
}

void hlw8110_write_reg_disable(void)
{
    hlw8110_write_reg(HLW8110_SPECIAL_REG,1,HLW8110_WRITE_DISABLE);
}

/*
* @input reg : register address
* @input len:  register package length (bytes) 
* @output data: package from reg 
* @return : os_result 
*/

os_err_t hlw8110_read_reg(os_uint8_t reg, os_uint8_t len, os_uint8_t* data)
{
    os_uint32_t size;
    os_uint8_t pbuf[64] = {0};
    static os_uint8_t resent_cnt = 0;
    os_uint8_t pbuf_len = 2 + len + 1;

    pbuf[0] = SMART_SOCKET_UART_HEADER;
    pbuf[1] = HLW8110_R_REG(reg);

    gc_serial_write(&pbuf[0],2);

    while (1)
    {
        if (os_mb_recv(&rx_mb, &size, 5) != OS_EOK){            //500ms
            if (resent_cnt < 3) {
                resent_cnt++;
                if (hlw8110_read_reg(reg,len,data) == OS_EOK) {
                    resent_cnt = 0;
                    return OS_EOK;
                }
            } else {
                resent_cnt = 0;
                SS_LOG(SS_LOG_LEVEL_ERR, "read reg timeout!\r\n");
                return OS_ETIMEOUT;
            }
        }
    
        os_uint8_t ret = gc_serial_read(&pbuf[2],64);
        if (ret < (len +1)) {
            SS_LOG(SS_LOG_LEVEL_ERR, "read reg error %d!\r\n",ret);
            return OS_ERROR;    
        }
        else
        {
            os_uint8_t tmp = hlw8110_checksum(&pbuf[0], pbuf_len -1);
            if( pbuf[pbuf_len -1] != tmp) {
                SS_LOG(SS_LOG_LEVEL_ERR, "read package checsum error! %02x  %02x\r\n",pbuf[pbuf_len -1],tmp);
                return OS_ERROR;
            }        
            
            memcpy(data,&pbuf[2],len);
			resent_cnt = 0;
            return OS_EOK;
        }        
    }
}

/*
* @input data: package from reg
* @input len:  byte of pakcage
* @return : actual value 
*/
os_uint32_t hlw8110_read_reg_package_to_actual_value(os_uint8_t* data, os_uint8_t len)
{
    os_uint32_t result = 0;
    
    for(int i = 0;i < len; i++)
    {
		result = result << 8;
        result = (result | data[i]);
    }
    return result;
}

/**
 * @brief read RmsIAC
 * @return actual RMS_IAC
 */ 
os_uint32_t hlw8110_read_rms_i_a_c(void)
{
    os_uint8_t pbuf[4] = {0};
    os_err_t ret = hlw8110_read_reg(HLW8110_RMS_I_A_C,2,pbuf);
    if (ret != OS_EOK) {
        SS_LOG(SS_LOG_LEVEL_ERR,"read error!\r\n");
        return OS_ERROR;
    }
    return hlw8110_read_reg_package_to_actual_value(pbuf,2);    
}

/**
 * @brief read RmsIBC
 * @return actual RmsIBC
 */ 
os_uint32_t hlw8110_read_rms_i_b_c(void)
{
    os_uint8_t pbuf[4] = {0};
    os_err_t ret = hlw8110_read_reg(HLW8110_RMS_I_B_C,2,pbuf);
    if (ret != OS_EOK) {
        SS_LOG(SS_LOG_LEVEL_ERR,"read error!\r\n");
        return OS_ERROR;
    }
    return hlw8110_read_reg_package_to_actual_value(pbuf,2);    
}
/**
 * @brief read RmsUC
 * @return actual RmsUC
 */ 
os_uint32_t hlw8110_read_rms_u_c(void)
{
    os_uint8_t pbuf[4] = {0};
    os_err_t ret = hlw8110_read_reg(HLW8110_RMS_U_C,2,pbuf);
    if (ret != OS_EOK) {
        SS_LOG(SS_LOG_LEVEL_ERR,"read error!\r\n");
        return OS_ERROR;
    }
    return hlw8110_read_reg_package_to_actual_value(pbuf,2);    
}
/**
 * @brief read power a c
 * @return actual power a c
 */ 
os_uint32_t hlw8110_read_power_a_c(void)
{
    os_uint8_t pbuf[4] = {0};
    os_err_t ret = hlw8110_read_reg(HLW8110_POWER_A_C,2,pbuf);
    if (ret != OS_EOK) {
        SS_LOG(SS_LOG_LEVEL_ERR,"read error!\r\n");
        return OS_ERROR;
    }
    return hlw8110_read_reg_package_to_actual_value(pbuf,2);    
}
/**
 * @brief read power b c
 * @return actual power b c
 */ 
os_uint32_t hlw8110_read_power_b_c(void)
{
    os_uint8_t pbuf[4] = {0};
    os_err_t ret = hlw8110_read_reg(HLW8110_POWER_B_C,2,pbuf);
    if (ret != OS_EOK) {
        SS_LOG(SS_LOG_LEVEL_ERR,"read error!\r\n");
        return OS_ERROR;
    }
    return hlw8110_read_reg_package_to_actual_value(pbuf,2);    
}
/**
 * @brief read power SC factor
 * @return actual power SC factor
 */ 
os_uint32_t hlw8110_read_power_sc(void)
{
    os_uint8_t pbuf[4] = {0};
    os_err_t ret = hlw8110_read_reg(HLW8110_POWER_SC,2,pbuf);
    if (ret != OS_EOK) {
        SS_LOG(SS_LOG_LEVEL_ERR,"read error!\r\n");
        return OS_ERROR;
    }
    return hlw8110_read_reg_package_to_actual_value(pbuf,2);    
}
/**
 * @brief read channel A energy factor
 * @return actual channel A energy factor
 */ 
os_uint32_t hlw8110_read_energy_a_c(void)
{
    os_uint8_t pbuf[4] = {0};
    os_err_t ret = hlw8110_read_reg(HLW8110_ENERGY_A_C,2,pbuf);
    if (ret != OS_EOK) {
        SS_LOG(SS_LOG_LEVEL_ERR,"read error!\r\n");
        return OS_ERROR;
    }
    return hlw8110_read_reg_package_to_actual_value(pbuf,2);    
}

/**
 * @brief read channel B energy factor
 * @return actual channel B energy factor
 */ 
os_uint32_t hlw8110_read_energy_b_c(void)
{
    os_uint8_t pbuf[4] = {0};
    os_err_t ret = hlw8110_read_reg(HLW8110_ENERGY_B_C,2,pbuf);
    if (ret != OS_EOK) {
        SS_LOG(SS_LOG_LEVEL_ERR,"read error!\r\n");
        return OS_ERROR;
    }
    return hlw8110_read_reg_package_to_actual_value(pbuf,2);    
}

/**
 * @brief read all factor checksum
 * @return  all factor checksum
 */ 
os_uint32_t hlw8110_read_all_factor_checksum(void)
{
    os_uint8_t pbuf[4] = {0};
    os_err_t ret = hlw8110_read_reg(HLW8110_ALL_FACTOR_CRC_REG,2,pbuf);
    if (ret != OS_EOK) {
        SS_LOG(SS_LOG_LEVEL_ERR,"read error!\r\n");
        return OS_ERROR;
    }
    return hlw8110_read_reg_package_to_actual_value(pbuf,2);    
}

/**
 * @brief confirm all factor are right
 */ 
os_err_t judge_hlw8100_factor_reg_checksum(void)
{
    hlw8110_actual_factor_t tmp_result;
	os_uint32_t result = 0;
   /* read RmsIAC、RmsIBC、RmsUC、PowerPAC、PowerPBC、PowerSC、EnergAc、EnergBc */
    result = hlw8110_read_rms_i_a_c();
	if ( result != OS_ERROR) {
		tmp_result.rms_i_a_c = result;
		result = 0;
    	SS_LOG(SS_LOG_LEVEL_TRACE,"channel A i factor %d\r\n " ,tmp_result.rms_i_a_c);
	}
    /*channel B factor*/
    result = hlw8110_read_rms_i_b_c();
	if ( result != OS_ERROR) {
		tmp_result.rms_i_b_c = result;
		result = 0;
		SS_LOG(SS_LOG_LEVEL_TRACE,"channel B i factor %d\r\n " ,tmp_result.rms_i_b_c);
	}
    /* rms u factor */
    result = hlw8110_read_rms_u_c();
	if ( result != OS_ERROR) {
		tmp_result.rms_u_c = result;
		result = 0;
		SS_LOG(SS_LOG_LEVEL_TRACE,"rms U factor %d\r\n " ,tmp_result.rms_u_c);
	}
    /* channel A power factor */
    result = hlw8110_read_power_a_c();
	if ( result != OS_ERROR) {
		tmp_result.power_a_c = result;
		result = 0;
		SS_LOG(SS_LOG_LEVEL_TRACE,"power a factor %d\r\n " ,tmp_result.power_a_c);
	}
    /* channel B power factor*/
    result = hlw8110_read_power_b_c();
	if ( result != OS_ERROR) {
		tmp_result.power_b_c = result;
		result = 0;
		SS_LOG(SS_LOG_LEVEL_TRACE,"power b factor %d\r\n " ,tmp_result.power_b_c);
	}
    /* SC power factor */    
    result = hlw8110_read_power_sc();
	if ( result != OS_ERROR) {
		tmp_result.power_sc = result;
		result = 0;
		SS_LOG(SS_LOG_LEVEL_TRACE,"power sc factor %d\n " ,tmp_result.power_sc);
	}
    /* channel A energy factor */
	result = hlw8110_read_energy_a_c();
	if ( result != OS_ERROR) {
		tmp_result.energy_a_c = result;
		result = 0;
		SS_LOG(SS_LOG_LEVEL_TRACE,"power sc factor %d\r\n " ,tmp_result.energy_a_c);
	}
    /*channel B energy factor*/
    result = hlw8110_read_energy_b_c();
	if ( result != OS_ERROR) {
		tmp_result.energy_b_c = result;
		result = 0;
		SS_LOG(SS_LOG_LEVEL_TRACE,"energy b factor %d\r\n " ,tmp_result.energy_b_c);
	}
    /*all factor register checksum*/
    result = hlw8110_read_all_factor_checksum();
	if ( result != OS_ERROR) {
		tmp_result.all_factor_reg_crc = result;
		result = 0;
		SS_LOG(SS_LOG_LEVEL_TRACE,"all factor register crc %d\r\n " ,tmp_result.all_factor_reg_crc);
	}

    os_uint64_t tmp_crc = (~(0xffff + tmp_result.rms_i_a_c + tmp_result.rms_i_b_c + 
    tmp_result.rms_u_c + tmp_result.energy_a_c + tmp_result.energy_b_c
     + tmp_result.power_a_c + tmp_result.power_b_c + tmp_result.power_sc)) & 0xffff;

    if ( tmp_crc == tmp_result.all_factor_reg_crc)
    {
        memcpy(&hlw8110_actual_factor,&tmp_result,sizeof(hlw8110_actual_factor_t));
		SS_LOG(SS_LOG_LEVEL_INFO,"checksum ok!\r\n");
        return OS_EOK;
    }
    else
    {
        SS_LOG(SS_LOG_LEVEL_ERR,"checksum error!\r\n");
        return OS_ERROR;
    }
  
    return OS_EOK;
}

os_err_t hlw8110_read_rms_a_i(void)
{    
    os_uint8_t pbuf[5] = {0};
    os_uint8_t len = 3;

    if (OS_EOK != hlw8110_read_reg(HLW8110_RMS_I_A_REG,len,pbuf)) {
        SS_LOG(SS_LOG_LEVEL_ERR,"read channel a RMS_i error\r\n");
        return OS_ERROR;
    }
    os_uint32_t ret = hlw8110_read_reg_package_to_actual_value(pbuf,len);
    hlw8110_actual_result.rms_i_a = hlw8110_rms_i_calculate(ret,hlw8110_actual_factor.rms_i_a_c);
    return OS_EOK;
}

os_err_t hlw8110_read_rms_u(void)
{    
    os_uint8_t pbuf[5] = {0};
    os_uint8_t len = 3;

    if (OS_EOK != hlw8110_read_reg(HLW8110_RMS_U_REG,len,pbuf)) {
        SS_LOG(SS_LOG_LEVEL_ERR,"read RMS_u error\r\n");
        return OS_ERROR;
    }
    os_uint32_t ret = hlw8110_read_reg_package_to_actual_value(pbuf,len);
    hlw8110_actual_result.rms_u = hlw8110_rms_u_calculate(ret,hlw8110_actual_factor.rms_u_c);
    
    return OS_EOK;
}

os_err_t hlw8110_read_power_a(void)
{    
    os_uint8_t pbuf[5] = {0};
    os_uint8_t len = 4;

    if (OS_EOK != hlw8110_read_reg(HLW8110_POWER_PA_REG,len,pbuf)) {
        SS_LOG(SS_LOG_LEVEL_ERR,"read channel a power error\r\n");
        return OS_ERROR;
    }
    os_uint32_t ret = hlw8110_read_reg_package_to_actual_value(pbuf,len);
    hlw8110_actual_result.power_a = hlw8110_power_calculate(ret,hlw8110_actual_factor.power_a_c);
    
    return OS_EOK;
}

os_err_t hlw8110_read_energy_a(void)
{
    os_uint8_t pbuf[5] = {0};
    os_uint8_t len = 3;

    if (OS_EOK != hlw8110_read_reg(HLW8110_ENERGY_PA_REG,len,pbuf)) {
        SS_LOG(SS_LOG_LEVEL_ERR,"read channel a energy error\r\n");
        return OS_ERROR;
    }
    os_uint32_t ret = hlw8110_read_reg_package_to_actual_value(pbuf,len);
    hlw8110_actual_result.energy_a = hlw8110_engergy_calculate(ret,hlw8110_actual_factor.energy_a_c) 
                                     + hlw8110_actual_result.base_energy_a;
    
    return OS_EOK;
}

void get_measurements_from_hlw8110(void* para)
{
    while (1) {
        hlw8110_read_rms_a_i();
        hlw8110_read_rms_u();
        hlw8110_read_power_a();
        hlw8110_read_energy_a();
        if(hlw8110_actual_result.energy_a != hlw8110_actual_result.bak_energy_a){
            SS_LOG(SS_LOG_LEVEL_INFO,"RMS_i = %d,RMS_u = %d,RMS_p = %d,RMS_e = %d\r\n",
                 hlw8110_actual_result.rms_i_a ,hlw8110_actual_result.rms_u,
                 hlw8110_actual_result.power_a,hlw8110_actual_result.energy_a);
                 hlw8110_actual_result.bak_energy_a = hlw8110_actual_result.energy_a;
        }
        
		os_task_msleep(800);		/*hlw8110 refresh frequency 3.4Hz*/
    }
}

/**
 * @brief hlw8110 result feedback
 * clean up inaccurate result when relay not open
 */
hlw8110_actual_result_t get_hlw8110_measurements(void)
{
#if 0
    relay_info_t relay_info;
    relay_info = get_smart_socket_relay_info();
    if (relay_info.on_off == RELAY_CLOSE) {
        hlw8110_actual_result.rms_i_a = 0;
        hlw8110_actual_result.rms_u = 0;
        hlw8110_actual_result.power_a = 0;
    } else if (hlw8110_actual_result.rms_i_a <= 20) {
        hlw8110_actual_result.rms_i_a = 0;
        hlw8110_actual_result.power_a = 0;
    }
#endif
    return hlw8110_actual_result;
}

int hlw8110_init(os_uint32_t energy_a)
{
    gs_hlw8110_init = OS_FALSE;
    gc_uart = gc_serial_open();
    if (NULL == gc_uart) {
        SS_LOG(SS_LOG_LEVEL_ERR,"gc serial open failed!\r\n");
        goto Exit;
    }        

    /* register write enable */
    hlw8110_write_reg_enable();
    os_task_msleep(5);
    /* disable all interrupts */
    hlw8110_write_reg(HLW8110_REG_IE_ADDR,2,0);
    os_task_msleep(10);
    /* channel A is selected*/   
    hlw8110_set_channel_a();
    os_task_msleep(10);
	/*set channel a, close channel B, voltage PGA = 1, current PGA=16*/
    hlw8110_write_reg(HLW8110_SYSCON_REG,2,HLW8110_CHANNEL_A_ENABLE);
    os_task_msleep(10);
    /*PFA Enable, pluse output enable, active power reg accumulation enable, active Zero crossing detection*/
    hlw8110_write_reg(HLW8110_EMUCON_REG,2,0x0181);
    /*EMUCON2 default value is 0x0001,waveEn = 1,zxEn = 1,Don't clear register 
    after reading channel A power.EPA_CB = 1, refresh frq 3.4Hz*/
    hlw8110_write_reg(HLW8110_EMUCON2_REG,2,0x0465);
    os_task_msleep(10);

    hlw8110_write_reg_disable();
    
    /* read register from 0x6F to 0x77 and check all crc */
    if (OS_EOK != judge_hlw8100_factor_reg_checksum()) {
        SS_LOG(SS_LOG_LEVEL_ERR,"hlw8100 reg checksum error!\r\n");
        goto Exit;
    }    
    /*new task to read regularly*/
    os_task_t *task;
    task = os_task_create("read_hlw8110", get_measurements_from_hlw8110, 
        OS_NULL,HLW8110_TASCK_SIZE,HLW8110_TASK_PRIORITY,HLW8110_TASK_TICK);
    if(task == OS_NULL)
        SS_LOG(SS_LOG_LEVEL_ERR,"create smart_socket task failed!\r\n");
    OS_ASSERT(task);
    os_task_startup(task);

    hlw8110_actual_result.base_energy_a = energy_a;
    hlw8110_actual_result.bak_energy_a = energy_a;
    SS_LOG(SS_LOG_LEVEL_ERR,"---------------------------------------  base_energy_a = %d !\r\n",energy_a);
    
    gs_hlw8110_init = OS_TRUE;

    return OS_EOK;
Exit:
    // os_mb_destroy(&rx_mb);
    gc_serial_close(gc_uart->dev);
    os_free(gc_uart);        
}

static void acw_power_plug_dump_func(int argc, char **argv)
{
    if ( OS_FALSE == gs_hlw8110_init)
    {
        os_kprintf("hlw8110 init failed, please check\r\n");
        return;
    }

    os_kprintf("RMS_i = %d,RMS_u = %d,RMS_p = %d,RMS_e = %d\r\n", hlw8110_actual_result.rms_i_a, hlw8110_actual_result.rms_u, 
        hlw8110_actual_result.power_a,hlw8110_actual_result.energy_a);

	return;
}

#ifdef OS_USING_SHELL
#include <shell.h>

SH_CMD_EXPORT(acw_power_plug_dump, acw_power_plug_dump_func, "acw_power_plug_dump test");
#endif
