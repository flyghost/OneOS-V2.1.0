/**
* @copyright Copyright (c) 2018 CMCC-CMIOT-SZ
*
* @file: gpio_common.h
* @brief:
* @version: 0.1
*
* @author: huzhou (huzhou@cmiot.chinamobile.com)
* @date: 2019.1.17
* @history:
* version  date | author | content
* 0.1 |2019.1.17 | huzhou | init
* 0.2 |2019.2.20 | huzhou | modify the GPIO definition for evt device and add the ant switch function
*
*
*/
#ifndef GPIO_COMMON_H
#define GPIO_COMMON_H

#include <core_cm4.h>
#include <nrf_gpio.h>
#include <nrf_delay.h>

#define GPIO_PIN_HIGH (1)
#define GPIO_PIN_LOW  (0)
#define NB_POWER_ON_DELAY 		(3000)
#define PERIPH_START_WAIT_DELAY (3000)
/***************************************************************************
* 			 UART PIN DEFINITION
***************************************************************************/
//nb UART
#define RX_PIN_NUM_NB 			(4)   //NB UART RECIVER
#define TX_PIN_NUM_NB 			(5)	//NB UART SEND
#define EN_NB_MODULE_PIN_NUM 	(23)   //FOR NEW
//gps UART
#define RX_PIN_NUM_GPS (8)   //GPS UART RECIVER
#define TX_PIN_NUM_GPS (7)	//GPS UART SEND
#define EN_PIN_NUM_GPS (22)

//WIFI UART
#define RX_PIN_NUM_WIFI (3)//9   //WIFI UART RECIVER
#define TX_PIN_NUM_WIFI (2)//10	//WIFI UART SEND

//ANT Switch
#define ANT_SEL_0_PIN_NUM  	(24)
#define ANT_SEL_1_PIN_NUM 	(20)

/***************************************************************************
* 			 NFC PIN DEFINITION
***************************************************************************/
#define NFC1_PIN_NUM    (9)
#define NFC2_PIN_NUM    (10)

/***************************************************************************
* 			 TWI PIN DEFINITION
***************************************************************************/
#define TWI_SDA_PIN_NUM (11)
#define TWI_SCL_PIN_NUM (12)

/***************************************************************************
* 			 SPI PIN DEFINITION
***************************************************************************/
#define SPI_CLK_PIN_NUM  (13)
#define SPI_SI_PIN_NUM   (14)
#define SPI_SO_PIN_NUM   (15)
#define SPI_CS_PIN_NUM   (16)
/***************************************************************************
* 			 LED PIN DEFINITION
***************************************************************************/
#define BLUE_LED_PIN_NUM  (27)
#define RED_LED_PIN_NUM   (28)
/***************************************************************************
* 			 BUTTON PIN DEFINITION
***************************************************************************/
#define SOS_BUTTON_PIN_NUM   (26)
/***************************************************************************
* 			 GSNESOR INT1 DEFINITION
***************************************************************************/
#define GSENSOR_INT_PIN_NUM   (31)
/***************************************************************************
* 			 BUZZER PIN DEFINITION
***************************************************************************/
#define BUZZER_GPIO_PIN_NUM   (25)
/***************************************************************************
* 			 POWER PIN DEFINITION
***************************************************************************/
#define CHARGE_CTRL_PIN_NUM          (30)
#define VCHARGE_DETECTION_PIN_NUM    (19)
#define CHARGING_CHK_PIN_NUM         (6)      //for charging status check

//only Battery quantity control for factory test
#define charge_threshold_control() do 				\
    { 												\
        nrf_gpio_cfg_output(ANT_SEL_0_PIN_NUM); 	\
        nrf_gpio_pin_write(ANT_SEL_0_PIN_NUM, GPIO_PIN_LOW); 	\
    }while(0)
/***************************************************************************
* 			 GPIO UART PIN DEFINITION
***************************************************************************/
//for new
#define GPIO_UART_TX_PIN_NUM    (17)
#define GPIO_UART_RX_PIN_NUM    (18)

/***************************************************************************
* 			 			ANT SWITCH
***************************************************************************/
__STATIC_INLINE
void app_gpio_output_cfg(uint32_t pin_number, nrf_gpio_pin_pull_t pull)
{
    nrf_gpio_cfg(
        pin_number,
        NRF_GPIO_PIN_DIR_OUTPUT,//NRF_GPIO_PIN_DIR_OUTPUT,
        NRF_GPIO_PIN_INPUT_DISCONNECT,
        pull,//NRF_GPIO_PIN_NOPULL,
        NRF_GPIO_PIN_S0S1,
        NRF_GPIO_PIN_NOSENSE);
}

__STATIC_INLINE
void app_gpio_cfg(uint32_t pin_number, nrf_gpio_pin_pull_t pull_type, nrf_gpio_pin_dir_t io_type)
{
    nrf_gpio_cfg(
        pin_number,
        io_type,
        NRF_GPIO_PIN_INPUT_DISCONNECT,
        pull_type,//NRF_GPIO_PIN_NOPULL,
        NRF_GPIO_PIN_S0S1,
        NRF_GPIO_PIN_NOSENSE);
}
/**
* @brief: function for switch ant to wifi
*
* @param:
* @return:
* @note:on hardware the function include the ANT switch and wifi enable
*/
__STATIC_INLINE
void app_wifi_ant_switch(void)
{
    nrf_gpio_cfg_output(GPIO_UART_TX_PIN_NUM);
    nrf_gpio_pin_write(GPIO_UART_TX_PIN_NUM, GPIO_PIN_HIGH);
    nrf_delay_ms(1);
    nrf_gpio_cfg_output(ANT_SEL_0_PIN_NUM);
    nrf_gpio_cfg_output(ANT_SEL_1_PIN_NUM);
    nrf_gpio_pin_write(ANT_SEL_0_PIN_NUM, GPIO_PIN_HIGH);
    nrf_gpio_pin_write(ANT_SEL_1_PIN_NUM, GPIO_PIN_LOW);
    nrf_delay_ms(PERIPH_START_WAIT_DELAY);
}
/**
* @brief: function for switch ant to ble
*
* @param:
* @return:
* @note:
*/
__STATIC_INLINE
void app_ble_ant_switch(void)
{
    nrf_gpio_cfg_output(ANT_SEL_0_PIN_NUM);
    nrf_gpio_cfg_output(ANT_SEL_1_PIN_NUM);
    nrf_gpio_cfg_output(GPIO_UART_TX_PIN_NUM);
    nrf_gpio_pin_write(GPIO_UART_TX_PIN_NUM, GPIO_PIN_LOW);
    nrf_gpio_pin_write(ANT_SEL_0_PIN_NUM, GPIO_PIN_LOW);
    nrf_gpio_pin_write(ANT_SEL_1_PIN_NUM, GPIO_PIN_HIGH);
}
/***************************************************************************
* 			 			NB module power control
***************************************************************************/
/**
* @brief: function for start nb module
*
* @param:
* @return:
* @note:called only when restart,when the system is running properly and uart not in NB port
*	, the nb module can be in the state of low power consumption.
*/
__STATIC_INLINE
void app_nb_module_power_on(void)
{
    nrf_gpio_cfg_output(EN_NB_MODULE_PIN_NUM);
    nrf_gpio_pin_write(EN_NB_MODULE_PIN_NUM, GPIO_PIN_HIGH);
    nrf_delay_ms(NB_POWER_ON_DELAY);
    nrf_gpio_pin_write(EN_NB_MODULE_PIN_NUM, GPIO_PIN_LOW);
    nrf_delay_ms(PERIPH_START_WAIT_DELAY);
}

/**
* @brief: function for nb module power off
*
* @param:
* @return:
* @note:called only when the device power off
*/
#define NB_POWER_OFF_DELAY (8500)
#define NB_WAKE_UP_DELAY   (100)
__STATIC_INLINE
void app_nb_module_power_off(void)
{
    nrf_gpio_pin_write(EN_NB_MODULE_PIN_NUM, GPIO_PIN_HIGH);
    nrf_delay_ms(NB_POWER_OFF_DELAY);
    nrf_gpio_pin_write(EN_NB_MODULE_PIN_NUM, GPIO_PIN_LOW);
}

__STATIC_INLINE
void app_nb_module_wake_up(void)
{
    nrf_gpio_pin_write(EN_NB_MODULE_PIN_NUM, GPIO_PIN_HIGH);
    nrf_delay_ms(NB_WAKE_UP_DELAY);
    nrf_gpio_pin_write(EN_NB_MODULE_PIN_NUM, GPIO_PIN_LOW);
}
/***************************************************************************
* 			 			GPS ENABLE
***************************************************************************/
/**
* @brief: function for gps power on
*
* @param:
* @return:
* @note:called only when the device power off
*/
__STATIC_INLINE
void app_gps_power_on(void)
{
    nrf_gpio_cfg_output(EN_PIN_NUM_GPS);
    nrf_gpio_pin_write(EN_PIN_NUM_GPS, GPIO_PIN_HIGH);
    nrf_delay_ms(PERIPH_START_WAIT_DELAY);
}
/**
* @brief: function for gps power off
*
* @param:
* @return:
* @note:called only when the device power off
*/
__STATIC_INLINE
void app_gps_power_off(void)
{
    nrf_gpio_cfg_output(EN_PIN_NUM_GPS);
    nrf_gpio_pin_write(EN_PIN_NUM_GPS, GPIO_PIN_LOW);
    //    nrf_delay_ms(PERIPH_START_WAIT_DELAY);
}

__STATIC_INLINE
void gpio_operation_before_system_off(void)
{
    //ANT SWITCH
    app_gpio_cfg(ANT_SEL_0_PIN_NUM, NRF_GPIO_PIN_PULLDOWN, NRF_GPIO_PIN_DIR_INPUT);
    app_gpio_cfg(ANT_SEL_1_PIN_NUM, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_DIR_INPUT);

    //config the pin to wake up the device form system off
    nrf_gpio_cfg_sense_input(CHARGING_CHK_PIN_NUM, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);

}
#endif

