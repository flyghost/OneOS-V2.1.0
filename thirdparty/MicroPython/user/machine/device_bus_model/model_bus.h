/**
 *********************************************************************************************************
 *                                                Micropython
 *
 *                              (c) Copyright CMCC IOT All Rights Reserved
 *
 * @file	: model_bus.h
 * @brief	: 总线头文件，总线模型的相关数据结构。
 * @author	: 周子涵
 * @version : V1.00.00
 * @date	: 2019年9月17日
 * 
 * The license and distribution terms for this file may be found in the file LICENSE in this distribution
*********************************************************************************************************
*/

#ifndef __BUS_H__
#define __BUS_H__

#include "model_core.h"
#include "model_def.h"
/*
*********************************************************************************************************
*                                        总线 类型数据结构  
*********************************************************************************************************
*/
enum bus_type
{
    PIN_BUS  =  0x0001,
    ADC_BUS  =  0x0002,
    I2C_BUS  =  0x0004,
    SPI_BUS  =  0x0008,
    UART_BUS =  0x0010,
};

/*
*********************************************************************************************************
*                                        总线操作接口  
*********************************************************************************************************
*/
struct  bus_operate
{
    int (*init)(void *device);
    int (*read)(void *device, void *buf, uint32_t bufsize);
    int (*write)(void *device, void *buf, uint32_t bufsize);
    int (*ioctl)(void *device, int cmd, void *arg);
    int (*deinit)(void *device);
    void (* callback)(void *arg);
};

/*
*********************************************************************************************************
*                                        gpio 数据结构 
*********************************************************************************************************
*/
struct pin_config{
    int         index;
    void        *grop;                      /*GPIO组(A,B,C,D,E...)                                     */
    uint32_t    pin;
};

struct pin_bus{
    struct core         *owner;             /*总线所有者，总线管理核心                                 */
    struct  pin_config  *map;
    struct  bus_operate *ops;               /*设备通用操作函数指针                                     */
};

/*
*********************************************************************************************************
*                                        adc 数据结构 
*********************************************************************************************************
*/
struct adc_config{
    uint32_t  channel;
    uint32_t  time;
};

struct adc_bus{
    struct core         *owner;             /*总线所有者，总线管理核心                                 */
    struct  adc_config  *cfg;
    struct  bus_operate *ops;               /*设备通用操作函数指针                                     */
};

/*
*********************************************************************************************************
*                                        i2c 数据结构 
*********************************************************************************************************
*/
struct i2c_client{
    uint8_t    addr;     	                /* 从机地址,7bit地址，1bit读写（0：w,1:r）          	     */
    uint8_t    irq; 	                    /* Interrupt priority.                           	 	   */
    uint16_t   flags; 	                    /*													 	   */
    uint32_t   len;      	                /* data length.	                                	 	   */
    void       *data;                       /* pointer to message data.							 	   */

};

struct i2c_adapter{
	int timeout;
	int retries;
	void *lock;
	int (*master_xfer)(struct i2c_adapter *adapt, struct i2c_client *msg, int num);
};

struct i2c_bus{
    struct core         *owner;             /*总线所有者，总线管理核心                                 */
    struct i2c_client   *client;
    struct i2c_adapter  *adapter;
    struct bus_operate  *ops;               /*设备通用操作函数指针                                     */
};



/*
*********************************************************************************************************
*                                        spi 数据结构 
*********************************************************************************************************
*/


struct  spi_client
{
    uint32_t len;		           /* data length.     	                                    	       */
    void     *data;  	           /* pointer to message data.                                	       */
    uint16_t flags;                      
    uint8_t  mode;                       
    #define SPI_DMA         BIT(1)   /* enable dma, the write function in the ops will use dma mode.   */
    #define SPI_DMA_DOUBLE	BIT(2)   /* Send twice in succession                                       */              
};             
               
struct adapter{            
    uint8_t speed_hz; 				/*               		                          	               */
	uint8_t mode;            		/* SPI mode.                                                       */
	#define    SPI_CPHA      0x01   /* clock phase 									 	               */
    #define    SPI_CPOL    	 0x02   /* clock polarity 									               */
    #define    SPI_MODE_0    (0|0)  /* (original MicroWire) 							               */
    #define    SPI_MODE_1    (0|SPI_CPHA)              
    #define    SPI_MODE_2    (SPI_CPOL|0)              
    #define    SPI_MODE_3    (SPI_CPOL|SPI_CPHA)               
    #define    SPI_CS_HIGH    0x04  /* chipselect active high? 						 	               */
    #define    SPI_LSB_FIRST  0x08  /* per-word bits-on-wire 							               */
    #define    SPI_3WIRE      0x10  /* SI/SO signals shared 							               */
    #define    SPI_LOOP       0x20  /* loopback mode 									               */
    #define    SPI_NO_CS      0x40  /* 1 dev/bus, no chipselect 						               */
    #define    SPI_READY      0x80  /* slave pulls low to pause 						               */
    uint8_t chip_select;     		/* chip select 										               */
	int     cs_gpio;			 	/* chip select gpio									               */
	void    *lock;
	int     irq;
};


struct spi_bus{
    struct core           *owner;   /* 总线所有者，总线管理核心                                        */
    struct bus_operate    *ops;     /* 设备通用操作函数指针                                            */
    struct spi_client 	  client;
    struct adapter		  master;
    int (*write_then_read)(void *bus, void *txbuf, uint32_t n_tx, void *rxbuf, uint32_t n_rx);
};

/*
*********************************************************************************************************
*                                       serial 数据结构 
*********************************************************************************************************
*/

struct model_serial_cfg
{
    uint32_t baud_rate;
    uint32_t data_bits;
    uint32_t stop_bits;
    uint32_t parity;
    uint32_t bit_order;
    uint32_t invert;
    uint32_t bufsz;
    uint32_t reserved;
};

struct serial_bus{
	struct core                 *owner;     /*总线所有者，总线管理核心                                 */
    struct  bus_operate         *ops;       /*设备通用操作函数指针                                     */
	struct  model_serial_cfg    cfg;
};

/*
*********************************************************************************************************
*                                        pwm 数据结构 
*********************************************************************************************************
*/
struct pwm_config
{
    uint8_t     duty;       /* 占空比.0-100*/
    uint8_t     channel;    /* 通道，0-n.*/
    float       frequency;  /* 频率. unit:hz*/
};

struct pwm_bus{
	struct  core            *owner;     /*总线所有者，总线管理核心                                 */
    struct  bus_operate     *ops;       /*设备通用操作函数指针                                     */
	struct  pwm_config      cfg;
	void                    *other;
};
#endif

