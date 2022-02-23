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
 * @file        drv_usart.c
 *
 * @brief       This file implements usart driver for gd32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <stdio.h>
#include <board.h>

#include <os_memory.h>
#include <bus/bus.h>
#include "drv_usart.h"
#include "drv_gpio.h"

typedef struct gd32_usart
{
    struct os_serial_device serial;
    
    const struct gd32_usart_info *info;

    soft_dma_t  sdma;
    os_uint32_t sdma_hard_size;
    
    dma_single_data_parameter_struct dma_init_struct;
    
    os_uint8_t *rx_buff;
    os_uint32_t rx_index;
    os_uint32_t rx_size;

    const os_uint8_t *tx_buff;
    os_uint32_t tx_count;
    os_uint32_t tx_size;

    os_list_node_t list; 
    
}gd32_usart_t;

static const struct gd32_usart_info *console_usart = OS_NULL;

static os_list_node_t gd32_usart_list = OS_LIST_INIT(gd32_usart_list);

static void gd32_usart_interrupt_rx(gd32_usart_t *usart)
{
    /* usart received irq */
    if (usart_interrupt_flag_get(usart->info->husart, USART_INT_FLAG_RBNE) != RESET)
    {
        OS_ASSERT(usart->rx_buff != OS_NULL);
        OS_ASSERT(usart->rx_index < usart->rx_size);
        
        usart_interrupt_flag_clear(usart->info->husart, USART_INT_FLAG_RBNE);

        usart->rx_buff[usart->rx_index++] = usart_data_receive(usart->info->husart);
    }

    /* usart idle irq */
    if (usart_interrupt_flag_get(usart->info->husart, USART_INT_FLAG_IDLE) != RESET)
    {
        usart_interrupt_flag_clear(usart->info->husart, USART_INT_FLAG_IDLE);
        
        usart_data_receive(usart->info->husart);
    
        if (usart->rx_index > 0)
        {
            /* soft dma idle irq */
            soft_dma_timeout_irq(&usart->sdma);
        }
    }

    /* soft dma half irq */
    if (usart->rx_index == (usart->rx_size / 2))
    {
        soft_dma_half_irq(&usart->sdma);
    }

    /* soft dma full irq */
    if (usart->rx_index == usart->rx_size)
    {
        usart->rx_index = 0;
        soft_dma_full_irq(&usart->sdma);
    }
}

static void gd32_usart_dma_rx(gd32_usart_t *usart)
{  
    /* usart received irq */
    if (usart_interrupt_flag_get(usart->info->husart, USART_INT_FLAG_RBNE) != RESET)
    {
        usart_interrupt_flag_clear(usart->info->husart, USART_INT_FLAG_RBNE);
        usart_data_receive(usart->info->husart);
    }

    /* usart idle irq */
    if (usart_interrupt_flag_get(usart->info->husart, USART_INT_FLAG_IDLE) != RESET)
    {
        usart_interrupt_flag_clear(usart->info->husart, USART_INT_FLAG_IDLE);
        usart_data_receive(usart->info->husart);

        /* soft dma idle irq */
        soft_dma_timeout_irq(&usart->sdma);
    }
}

static void gd32_usart_irq_callback(gd32_usart_t *usart)
{
    /* rx */
    if (usart->info->dma_periph == OS_NULL)
    {
        gd32_usart_interrupt_rx(usart);
    }
    else
    {
        gd32_usart_dma_rx(usart);
    }

    /* tx */
    if (usart_interrupt_flag_get(usart->info->husart, USART_INT_FLAG_TBE) != RESET)
    {
        usart_interrupt_flag_clear(usart->info->husart, USART_INT_FLAG_TBE);
        
        if (usart->tx_size > 0)
        {
            if (usart->tx_count < usart->tx_size)
                usart_data_transmit(usart->info->husart, usart->tx_buff[usart->tx_count++]);

            if (usart->tx_count >= usart->tx_size)
            {
                usart->tx_size = 0;
                usart_interrupt_disable(usart->info->husart, USART_INT_TBE);
                os_hw_serial_isr_txdone((struct os_serial_device *)usart);
            }
        }
    }
}

static void usart_irq_handler(void)
{
    struct gd32_usart *usart;

    os_list_for_each_entry(usart, &gd32_usart_list, struct gd32_usart, list)
    {
        gd32_usart_irq_callback(usart);
        return;
    }   
}

static void usart_dma_irqhandler(void)
{
    struct gd32_usart *usart;
    
    os_list_for_each_entry(usart, &gd32_usart_list, struct gd32_usart, list)
    {
        soft_dma_half_irq(&usart->sdma);
        return;    
    }
}


void USART0_IRQHandler(void)
{
    usart_irq_handler();  
}

void USART1_IRQHandler(void)
{
    usart_irq_handler();  
}

void USART2_IRQHandler(void)
{
    usart_irq_handler();
}

/* USART0 */
void DMA1_Channel2_IRQHandler(void)
{
    if (SET == dma_flag_get(DMA1, DMA_CH2, DMA_FLAG_HTF))
    {   
        dma_interrupt_flag_clear(DMA1, DMA_CH2, DMA_FLAG_HTF);   
    }
    
    if (SET == dma_flag_get(DMA1, DMA_CH2, DMA_FLAG_FTF))
    {
        dma_interrupt_flag_clear(DMA1, DMA_CH2, DMA_FLAG_FTF);   
    }

    usart_dma_irqhandler();
}

static os_uint32_t gd32_sdma_int_get_index(soft_dma_t *dma)
{
    gd32_usart_t *usart = os_container_of(dma, gd32_usart_t, sdma);

    return usart->rx_index;
}

static os_err_t gd32_sdma_int_start(soft_dma_t *dma, void *buff, os_uint32_t size)
{
    gd32_usart_t *usart = os_container_of(dma, gd32_usart_t, sdma);

    usart->rx_buff  = buff;
    usart->rx_index = 0;
    usart->rx_size  = size;

    usart_interrupt_enable(usart->info->husart, USART_INT_RBNE);
    
    return OS_EOK;
}

static os_uint32_t gd32_sdma_int_stop(soft_dma_t *dma)
{
    gd32_usart_t *usart = os_container_of(dma, gd32_usart_t, sdma);

    usart_interrupt_disable(usart->info->husart, USART_INT_RBNE);
    usart_interrupt_disable(usart->info->husart, USART_INT_IDLE);
    
    return gd32_sdma_int_get_index(dma);
}

static os_uint32_t gd32_sdma_dma_get_index(soft_dma_t *dma)
{
    gd32_usart_t *usart = os_container_of(dma, gd32_usart_t, sdma);

    return usart->sdma_hard_size - dma_transfer_number_get((uint32_t)(usart->info->dma_periph), usart->info->dma_channel);
}

static os_err_t gd32_sdma_dma_init(soft_dma_t *dma)
{ 
    return OS_EOK;
}

static os_err_t gd32_sdma_dma_start(soft_dma_t *dma, void *buff, os_uint32_t size)
{
    gd32_usart_t *usart = os_container_of(dma, gd32_usart_t, sdma);
     
    usart->sdma_hard_size = size;
    
    /* enable DMA clock */
    rcu_periph_clock_enable(RCU_DMA1); 
 
    /* DMA deinit */
    dma_deinit((uint32_t)usart->info->dma_periph, usart->info->dma_channel);
    dma_single_data_para_struct_init(&usart->dma_init_struct);
    
    /* set dma init parameters */
    usart->dma_init_struct.direction = DMA_PERIPH_TO_MEMORY;
    usart->dma_init_struct.periph_addr = (uint32_t)&USART_DATA(usart->info->husart);
    usart->dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    usart->dma_init_struct.memory0_addr = (uint32_t)buff;
    usart->dma_init_struct.periph_memory_width = DMA_MEMORY_WIDTH_8BIT;
    usart->dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    usart->dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    usart->dma_init_struct.number = size;    
       
    /* DMA init */
    dma_single_data_mode_init((uint32_t)usart->info->dma_periph, usart->info->dma_channel, &usart->dma_init_struct);
 
    /* configure DMA mode */
    dma_circulation_enable((uint32_t)usart->info->dma_periph, usart->info->dma_channel); 

    /* configure DMA periphral */
    dma_channel_subperipheral_select(DMA1, DMA_CH2, DMA_SUBPERI4);

    /* USART DMA enable for receive */
    usart_dma_receive_config(usart->info->husart, USART_DENR_ENABLE);

    /* enable DMA interrupt*/   
    if (usart->info->husart == USART0)
    {
        nvic_irq_enable(DMA1_Channel2_IRQn, 0, 0);
    }

    /* enable DMA1 channel2 */
    dma_channel_enable((uint32_t)usart->info->dma_periph, usart->info->dma_channel); 

    /* enable USART interrupt */
    usart_interrupt_enable(usart->info->husart, USART_INT_IDLE);
    usart_interrupt_enable(usart->info->husart, USART_INT_RBNE);

    /* enable DMA transfer half interrupt */
    dma_interrupt_enable((uint32_t)usart->info->dma_periph, usart->info->dma_channel, DMA_INTF_HTFIF);

    /* enable DMA transfer complete interrupt */
    dma_interrupt_enable((uint32_t)usart->info->dma_periph, usart->info->dma_channel, DMA_INTF_FTFIF);

    return OS_EOK;
}

static os_uint32_t gd32_sdma_dma_stop(soft_dma_t *dma)
{
    gd32_usart_t *usart = os_container_of(dma, gd32_usart_t, sdma);

    /* disable DMA */
    dma_channel_disable((uint32_t)usart->info->dma_periph, usart->info->dma_channel);;

    return gd32_sdma_dma_get_index(dma);
}

/* sdma callback */
static void gd32_usart_sdma_callback(soft_dma_t *dma)
{
    gd32_usart_t *uart = os_container_of(dma, gd32_usart_t, sdma);

    os_hw_serial_isr_rxdone((struct os_serial_device *)uart);
}

static void gd32_usart_sdma_init(struct gd32_usart *usart, dma_ring_t *ring)
{
    soft_dma_t *dma = &usart->sdma;

    soft_dma_stop(dma);

    memset(&dma->hard_info, 0, sizeof(dma->hard_info));


    dma->hard_info.mode         = HARD_DMA_MODE_CIRCULAR;
    dma->hard_info.max_size     = 64 * 1024;
    dma->hard_info.data_timeout = uart_calc_byte_timeout_us(usart->serial.config.baud_rate);
    dma->hard_info.flag         = HARD_DMA_FLAG_HALF_IRQ | HARD_DMA_FLAG_FULL_IRQ | HARD_DMA_FLAG_TIMEOUT_IRQ;
    if (usart->info->dma_periph == OS_NULL)
    {  
        dma->ops.get_index          = gd32_sdma_int_get_index;
        dma->ops.dma_init           = OS_NULL;
        dma->ops.dma_start          = gd32_sdma_int_start;
        dma->ops.dma_stop           = gd32_sdma_int_stop;
    }
    else
    {
        dma->ops.get_index          = gd32_sdma_dma_get_index;
        dma->ops.dma_init           = gd32_sdma_dma_init;
        dma->ops.dma_start          = gd32_sdma_dma_start;
        dma->ops.dma_stop           = gd32_sdma_dma_stop;
    }

    dma->cbs.dma_half_callback      = gd32_usart_sdma_callback;
    dma->cbs.dma_full_callback      = gd32_usart_sdma_callback;
    dma->cbs.dma_timeout_callback   = gd32_usart_sdma_callback;

    soft_dma_init(dma);
    soft_dma_start(dma, ring);
    soft_dma_irq_enable(&usart->sdma, OS_TRUE);
}

static int __gd32_usart_init(const struct gd32_usart_info *usart_info, struct serial_configure *cfg)
{
    /* enable GPIO clock */
    rcu_periph_clock_enable((rcu_periph_enum)usart_info->pin_clk);

    /* enable USART clock */
    rcu_periph_clock_enable((rcu_periph_enum)usart_info->usart_clk);

    /* connect port to USARTx_Tx */
    gpio_af_set(usart_info->pin_port, usart_info->gpio_af_idx, usart_info->tx_pin);

    /* connect port to USARTx_Rx */
    gpio_af_set(usart_info->pin_port, usart_info->gpio_af_idx, usart_info->rx_pin);
    
    /* configure USART Tx as alternate function push-pull */
    gpio_mode_set(usart_info->pin_port, GPIO_MODE_AF, GPIO_PUPD_PULLUP, usart_info->tx_pin);
    gpio_output_options_set(usart_info->pin_port, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, usart_info->tx_pin);

    /* configure USART Rx as alternate function push-pull */
    gpio_mode_set(usart_info->pin_port, GPIO_MODE_AF, GPIO_PUPD_PULLUP, usart_info->rx_pin);
    gpio_output_options_set(usart_info->pin_port, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, usart_info->rx_pin);

    /* USART configure */
    usart_deinit(usart_info->husart);
    usart_baudrate_set(usart_info->husart, cfg->baud_rate);
    usart_word_length_set(usart_info->husart, cfg->data_bits);
    usart_stop_bit_set(usart_info->husart, cfg->stop_bits);
    usart_parity_config(usart_info->husart, cfg->parity);
    usart_hardware_flow_rts_config(usart_info->husart, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(usart_info->husart, USART_CTS_DISABLE);   
    usart_receive_config(usart_info->husart, USART_RECEIVE_ENABLE);
    usart_transmit_config(usart_info->husart, USART_TRANSMIT_ENABLE);
    usart_enable(usart_info->husart);

    return OS_EOK;       
}

static os_err_t gd32_usart_init(struct os_serial_device *serial, struct serial_configure *cfg)
{
    struct gd32_usart *usart;

    OS_ASSERT(serial != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);

    usart = os_container_of(serial, struct gd32_usart, serial);

    __gd32_usart_init(usart->info, cfg);

    nvic_irq_enable(usart->info->irq, 0, 0);

    /* Start RX */
    usart_interrupt_enable(usart->info->husart, USART_INT_RBNE);
    usart_interrupt_enable(usart->info->husart, USART_INT_IDLE);    
    
    /* SOFT DMA Init */
    gd32_usart_sdma_init(usart, &serial->rx_fifo->ring);

    return OS_EOK;
}

static os_err_t gd32_usart_deinit(struct os_serial_device *serial)
{
    struct gd32_usart *usart;

    OS_ASSERT(serial != OS_NULL);

    usart = os_container_of(serial, struct gd32_usart, serial);

    /* rx */
    usart_interrupt_disable(usart->info->husart, USART_INT_RBNE);
    usart_interrupt_disable(usart->info->husart, USART_INT_IDLE);    

    /* usart dma deinit */
    if (usart->info->dma_periph != OS_NULL)
    {
        /* soft dma deinit */
        soft_dma_stop(&usart->sdma);
        
        /* hard dma deinit */
        dma_deinit((uint32_t)usart->info->dma_periph, usart->info->dma_channel);
    }

    /* tx */
    usart_interrupt_disable(usart->info->husart, USART_INT_TBE);
    
    usart->tx_buff  = OS_NULL;
    usart->tx_count = 0;
    usart->tx_size  = 0;

    return 0;
}

static int gd32_usart_start_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    struct gd32_usart *usart;

    OS_ASSERT(serial != OS_NULL);

    usart = os_container_of(serial, struct gd32_usart, serial);

    usart->tx_buff  = buff;
    usart->tx_count = 0;
    usart->tx_size  = size;

    usart_interrupt_enable(usart->info->husart, USART_INT_TBE);

    return size;
}

static int gd32_usart_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    struct gd32_usart *uart;
    os_size_t i;
    os_base_t level;

    OS_ASSERT(serial != OS_NULL);
    
    uart = os_container_of(serial, struct gd32_usart, serial);

    for (i = 0; i < size; i++)
    {
        level = os_irq_lock();
        usart_data_transmit(uart->info->husart, buff[i]);
        while (RESET == usart_flag_get(uart->info->husart, USART_FLAG_TBE));
        os_irq_unlock(level);
    }
    
    return size;
}

static const struct os_uart_ops gd32_usart_ops = {
    .init         = gd32_usart_init,
    .deinit       = gd32_usart_deinit,
    
    .start_send   = gd32_usart_start_send,
    .poll_send    = gd32_usart_poll_send,
};

int gd32_usart_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{     
    struct serial_configure config  = OS_SERIAL_CONFIG_DEFAULT;

    os_err_t    result  = 0;
    os_base_t   level;    
    
    struct gd32_usart *uart = os_calloc(1, sizeof(struct gd32_usart));

    OS_ASSERT(uart);

    uart->info = dev->info;

    struct os_serial_device *serial = &uart->serial;

    serial->ops    = &gd32_usart_ops;
    serial->config = config;

    level = os_irq_lock();
    os_list_add_tail(&gd32_usart_list, &uart->list);
    os_irq_unlock(level);

    result = os_hw_serial_register(serial, dev->name, NULL);

    OS_ASSERT(result == OS_EOK);

    return result;    
}

void __os_hw_console_output(char *str)
{
    int i;
    
    if (console_usart == OS_NULL)
        return;

    for (i = 0; i < strlen(str); i++)
    {
        while (usart_flag_get(console_usart->husart, USART_FLAG_TC) == RESET);
            usart_data_transmit(console_usart->husart, str[i]);
    }
}

OS_DRIVER_INFO gd32_usart_driver = {
    .name   = "Usart_Type",
    .probe  = gd32_usart_probe,
};

OS_DRIVER_DEFINE(gd32_usart_driver, PREV, OS_INIT_SUBLEVEL_HIGH);


#include "string.h"
static int gd32_uart_early_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{   
    if(strcmp(dev->name, OS_CONSOLE_DEVICE_NAME))
        return OS_EOK;
    
    console_usart = (struct gd32_usart_info *)dev->info;
    
    struct serial_configure config  = OS_SERIAL_CONFIG_DEFAULT;
    
    __gd32_usart_init(console_usart, &config);
    
    return OS_EOK;    
}   

OS_DRIVER_INFO gd32_uart_early_driver = {
    .name   = "Usart_Type",
    .probe  = gd32_uart_early_probe,
};

OS_DRIVER_DEFINE(gd32_uart_early_driver, CORE, OS_INIT_SUBLEVEL_LOW);

