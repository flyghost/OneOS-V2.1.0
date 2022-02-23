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
 * @file        drv_uart.c
 *
 * @brief       This file implements usart driver for beken
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */


#include <os_hw.h>
#include <os_task.h>
#include <os_device.h>
#include <os_memory.h>

#include "bus.h"
#include "os_assert.h"
#include "serial.h"
#include "interrupt.h"
#include "typedef.h"
#include "drv_uart.h"
#include "uart.h"
#include "icu_pub.h"
#include "board.h"
#include "drv_model_pub.h"
#include "arm_arch.h"

#ifdef OS_USING_SERIAL 

struct device_uart
{
    struct os_serial_device serial;
    os_uint32_t port;
    os_uint32_t irqno; 
	struct beken_uart_info *uart_info;
	os_uint8_t   *buff;
	os_uint32_t   count;
	os_uint32_t   size;
	os_uint32_t   state;
    char name[OS_NAME_MAX];
};

static struct device_uart *g_uart[2];


void beken_uart_isr_rx(unsigned char uport)
{
	struct os_serial_device *serial = OS_NULL;
	struct device_uart * uart = OS_NULL;
	unsigned int reg_addr	 = 0;
	unsigned int reg_val	 = 0;
	unsigned int fifo_status = 0;

    if(UART1_PORT == uport) 
    {
    	uart    	= g_uart[0];
        fifo_status = REG_UART1_FIFO_STATUS;
		reg_val  	= REG_READ(REG_UART1_INTR_ENABLE);
		reg_addr 	= REG_UART1_INTR_ENABLE;
    }
    else
    {
		uart  		= g_uart[1];
		fifo_status = REG_UART2_FIFO_STATUS;
		reg_val  	= REG_READ(REG_UART2_INTR_ENABLE);
		reg_addr	= REG_UART2_INTR_ENABLE;
    }
  
    OS_ASSERT(uart != OS_NULL);

	if(1 == uart->state)
	{
		while(REG_READ(fifo_status) & FIFO_RD_READY)
		{ // read all HW fifo data
		    if(uart->count < uart->size)
			{
		   		uart->buff[uart->count] = (os_uint8_t)uart_read_byte(uport);
				uart->count ++;
			}
		    else
		    {
		        break;
		    }
		}

		if(uart->count == uart->size)
		{
			uart->state = 0;
			
			reg_val &= ~(RX_FIFO_NEED_READ_EN | UART_RX_STOP_END_EN);
			REG_WRITE(reg_addr,reg_val);
		}
	}
}

static os_err_t beken_uart_configure(struct os_serial_device *serial, struct serial_configure *cfg)
{
    os_uint32_t addr, val;
    struct device_uart * uart;
    bk_uart_config_t config;

    OS_ASSERT(serial != OS_NULL);
    serial->config = *cfg;

    uart = serial->parent.user_data;
    OS_ASSERT(uart != OS_NULL);

    OS_ASSERT((serial->config.data_bits >= DATA_BITS_5) && \
              (serial->config.data_bits <= DATA_BITS_8));

    config.baud_rate = serial->config.baud_rate;
    config.data_width = serial->config.data_bits - DATA_BITS_5;
    config.parity = serial->config.parity;
    config.stop_bits = serial->config.stop_bits;
    config.flow_control = FLOW_CTRL_DISABLED;
    config.flags = 0;
    
    uart_hw_set_change(uart->port, &config);

    return OS_EOK;
}

static int beken_uart_start_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
	return OS_EOK;
}

static int beken_uart_stop_send(struct os_serial_device *serial)
{
	return OS_EOK;
}

static int beken_uart_start_recv(struct os_serial_device *serial, os_uint8_t *buff, os_size_t size)
{
    unsigned int len   		 = 0;
    unsigned int uport 		 = 0;
    unsigned int reg_val     = 0;
    unsigned int reg_addr	 = 0;
    int          ret 		 = 0;
    struct device_uart *uart = OS_NULL;
    unsigned int param;

    OS_ASSERT(serial != OS_NULL);
    uart  = serial->parent.user_data;
    OS_ASSERT(uart != OS_NULL);
    uport = uart->port;

    if (UART1_PORT == uport)
    {
        reg_val  = REG_READ(REG_UART1_INTR_ENABLE);
        reg_addr = REG_UART1_INTR_ENABLE;
        param = IRQ_UART1_BIT;
    }
    else
    {
        reg_val  = REG_READ(REG_UART2_INTR_ENABLE);
        reg_addr = REG_UART2_INTR_ENABLE;
        param = IRQ_UART2_BIT;
    }

    uart->state = 1;
    uart->buff	= buff;
    uart->count = 0;
    uart->size	= size;

    reg_val |= (RX_FIFO_NEED_READ_EN | UART_RX_STOP_END_EN);
    REG_WRITE(reg_addr,reg_val);

    sddev_control(ICU_DEV_NAME, CMD_ICU_INT_ENABLE, &param);

    return ret;
}


static int beken_uart_stop_recv(struct os_serial_device *serial)
{
 	unsigned int len   		 = 0;
 	unsigned int uport 		 = 0;
 	unsigned int reg_val     = 0;
 	unsigned int reg_addr	 = 0;
 	int          ret 		 = 0;
	struct device_uart *uart = OS_NULL;

	
	OS_ASSERT(serial != OS_NULL);
	uart  = serial->parent.user_data;
	OS_ASSERT(uart != OS_NULL);
	uport = uart->port;

	if (UART1_PORT == uport)
	{
		reg_val  = REG_READ(REG_UART1_INTR_ENABLE);
		reg_addr = REG_UART1_INTR_ENABLE;
	}
	else
	{
		reg_val  = REG_READ(REG_UART2_INTR_ENABLE);
		reg_addr = REG_UART2_INTR_ENABLE;
	}

	uart->state = 0;
	uart->buff	= OS_NULL;
	reg_val &= ~(RX_FIFO_NEED_READ_EN | UART_RX_STOP_END_EN);
	
	REG_WRITE(reg_addr,reg_val);
	
	return ret;
}


static int beken_uart_recv_state(struct os_serial_device *serial)
{
	int state = 0;
	unsigned int fifo_status_reg = 0;
	struct device_uart *uart	 = OS_NULL;
	os_base_t level;

	OS_ASSERT(serial != OS_NULL);
	uart  = serial->parent.user_data;
	OS_ASSERT(uart != OS_NULL);

	state = uart->count;
	if(0 == uart->state)
		state |= OS_SERIAL_FLAG_RX_IDLE;

	return state;
}

static int beken_uart_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{ 
	struct device_uart *uart = OS_NULL;
    unsigned int len  = 0;
	unsigned int i    = 0;
	os_base_t level;
	
    OS_ASSERT(serial != OS_NULL);
    uart = serial->parent.user_data;
	OS_ASSERT(uart != OS_NULL);

	level = os_hw_interrupt_disable();
	for(i = 0; i < size; ++i)
	{	    
		 if(uart_write_byte(uart->port,buff[i]));
	    	++len;
	}
	os_hw_interrupt_enable(level);

    return (size > len) ? len : size ;
}

static int beken_uart_poll_recv(struct os_serial_device *serial, os_uint8_t *buff, os_size_t size)
{
	struct device_uart *uart = OS_NULL;
	unsigned int i    = 0;
	os_base_t level;
	
    OS_ASSERT(serial != OS_NULL);
    uart = serial->parent.user_data;
	OS_ASSERT(uart != OS_NULL);

	level = os_hw_interrupt_disable();
	for(i = 0; i < size; ++i)
	{	    
		 buff[i] = (os_uint8_t)uart_read_byte(uart->port); 
	}
	os_hw_interrupt_enable(level);

    return size ;
}


static const struct os_uart_ops beken_uart_ops = {
    .configure    = beken_uart_configure,

    .start_send   = OS_NULL,//beken_uart_start_send
    .stop_send    = OS_NULL,//beken_uart_stop_send

    .start_recv   = beken_uart_start_recv,
    .stop_recv    = beken_uart_stop_recv,
    .recv_state   = beken_uart_recv_state,
    
    .poll_send    = beken_uart_poll_send,
    .poll_recv    = beken_uart_poll_recv,
};



static int beken_uart_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct serial_configure config  = OS_SERIAL_CONFIG_DEFAULT;
    
    os_err_t    result  = 0;
    os_base_t   level;

    struct device_uart *uart = os_calloc(1, sizeof(struct device_uart));
    OS_ASSERT(uart);

    uart->uart_info = (struct beken_uart_info *)dev->info;

    struct os_serial_device *serial = &uart->serial;

    serial->ops    = &beken_uart_ops;
    serial->config = config;
	
    uart->port  = uart->uart_info->port;   // UART_PORT;
    uart->irqno = uart->uart_info->irqno;  // IRQ_UART;
    os_memcpy(uart->name, dev->name,sizeof(dev->name));

	if (UART1_PORT == uart->port)
		g_uart[0] = uart;
	else
		g_uart[1] = uart;

    result = os_hw_serial_register(serial, dev->name, OS_DEVICE_FLAG_RDWR, uart);
    OS_ASSERT(result == OS_EOK);

    return result;
}

OS_DRIVER_INFO beken_uart_driver = {
    .name   = "Uart_Type",
    .probe  = beken_uart_probe,
};

OS_DRIVER_DEFINE(beken_uart_driver, "0.end.0");
#endif
