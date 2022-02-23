/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the \"License\ you may not use this file except in compliance with 
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on 
 * an \"AS IS\" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the 
 * specific language governing permissions and limitations under the License.
 *
 * \@file        MQTTOneOS.c
 *
 * \@brief       socket port file for mqtt
 *
 * \@details     
 *
 * \@revision
 * Date         Author          Notes
 * 2020-06-08   OneOS Team      first version
 ***********************************************************************************************************************
 */

#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <pin.h>
#include <os_util.h>
#include <drv_gpio.h>

#include "led_key.h"

#ifdef BOARD_BK7231N
#define SW_GREEN_LED_P21 		(17) 		//绿灯
#define SW_RED_LED_P20 		    (28) 		//红灯
#define SW_RELAY_P20			(20)
#define SW_KEY_ON_OFF_PIN		(21)
#endif

static os_bool_t gs_led_io_inited = OS_FALSE;
static os_bool_t gs_key_io_inited = OS_FALSE;

typedef enum
{
	LED_RED = 0,
	LED_GREEN,
	LED_BLUE
} acw_led_index_t;

typedef struct
{
    os_base_t pin_base;
    int press_cnt;
	int free_value;
} acw_key_clr_t;

static acw_key_clr_t gs_acw_key_clr;

void acw_powerplug_open(void)
{
#ifdef BOARD_BK7231N
	os_pin_write(SW_RELAY_P20, PIN_HIGH);
#endif
}

void acw_powerplug_close(void)
{
#ifdef BOARD_BK7231N
	os_pin_write(SW_RELAY_P20, PIN_LOW);
#endif
}

void acw_led_io_init(void)
{
#ifdef OS_USING_LED
	if (led_table_size < LED_GREEN)
	{
		return;
	}

    for (int i = 0; i < led_table_size; i++)
    {
        os_pin_mode(led_table[i].pin, PIN_MODE_OUTPUT);
		os_pin_write(led_table[i].pin, PIN_HIGH);
    }
	gs_led_io_inited = OS_TRUE;
#endif

#ifdef BOARD_BK7231N
	os_pin_mode(SW_RELAY_P20, PIN_MODE_OUTPUT);
	os_pin_write(SW_RELAY_P20, PIN_LOW);		// relay off

	os_pin_mode(SW_GREEN_LED_P21, PIN_MODE_OUTPUT);
	os_pin_write(SW_GREEN_LED_P21, PIN_HIGH);	// LED off

	os_pin_mode(SW_RED_LED_P20, PIN_MODE_OUTPUT);
	os_pin_write(SW_RED_LED_P20, PIN_HIGH);		// LED off

	gs_led_io_inited = OS_TRUE;
#endif
}

#ifdef BOARD_BK7231N
static void bk_pin_callback(void *args)
{
	return;
}
#endif

#ifdef OS_USING_PUSH_BUTTON
static void acw_pin_callback(void *args)
{
	int index;
	int key;

    key = (int)(unsigned long)args;

	for (index = 0; index < key_table_size; index++)
	{
		if (key_table[index].pin == key)
		{
			break;
		}
	}

	os_kprintf("KEY:%d press\r\n", index);

	return;
}
#endif

void acw_key_io_init(void)
{
#ifdef OS_USING_PUSH_BUTTON
	if (key_table_size <= 0)
	{
		return;
	}

    for (int i = 0; i < key_table_size; i++)
    {
        os_pin_mode(key_table[i].pin, key_table[i].mode);
		os_pin_attach_irq(key_table[i].pin, key_table[i].irq_mode, acw_pin_callback, (void *)key_table[i].pin);
        os_pin_irq_enable(key_table[i].pin, PIN_IRQ_ENABLE);
    }
	
	gs_acw_key_clr.pin_base = key_table[0].pin;
	gs_acw_key_clr.press_cnt = 0;
	//if the board will reboot, to check this if correct
	gs_acw_key_clr.free_value = os_pin_read(gs_acw_key_clr.pin_base);

	gs_key_io_inited = OS_TRUE;
#endif

#ifdef BOARD_BK7231N
	os_pin_mode(SW_KEY_ON_OFF_PIN, PIN_MODE_INPUT);
  	os_pin_attach_irq(SW_KEY_ON_OFF_PIN, PIN_IRQ_MODE_FALLING, bk_pin_callback, OS_NULL);
	os_pin_irq_enable(SW_KEY_ON_OFF_PIN, PIN_IRQ_ENABLE);

	gs_acw_key_clr.pin_base = SW_KEY_ON_OFF_PIN;
	gs_acw_key_clr.press_cnt = 0;
	gs_acw_key_clr.free_value = PIN_HIGH;

	gs_key_io_inited = OS_TRUE;
#endif
}

os_bool_t acw_check_clr_key_press(void)
{
	int key_value;

	if (OS_FALSE == gs_key_io_inited)
	{
		return OS_FALSE;
	}

	key_value = os_pin_read(gs_acw_key_clr.pin_base);
	if (gs_acw_key_clr.free_value != key_value)
	{
		gs_acw_key_clr.press_cnt++;
	}
	else
	{
		gs_acw_key_clr.press_cnt = 0;
	}

	if (gs_acw_key_clr.press_cnt > 2)
	{
		os_kprintf("Find clr key press\r\n");
		return OS_TRUE;
	}

	return OS_FALSE;
}

void acw_red_led_open(void)
{
	if (OS_FALSE ==  gs_led_io_inited)
	{
		return;
	}

#ifdef OS_USING_LED	
	os_pin_write(led_table[LED_GREEN].pin, PIN_HIGH);
	os_pin_write(led_table[LED_RED].pin, PIN_LOW);
#endif

#ifdef BOARD_BK7231N
	os_pin_write(SW_RED_LED_P20, PIN_HIGH);
	os_pin_write(SW_GREEN_LED_P21, PIN_LOW);
#endif
	return;
}

void acw_red_led_close(void)
{
	if (OS_FALSE ==  gs_led_io_inited)
	{
		return;
	}

#ifdef OS_USING_LED	
	os_pin_write(led_table[LED_GREEN].pin, PIN_HIGH);
	os_pin_write(led_table[LED_RED].pin, PIN_HIGH);
#endif

#ifdef BOARD_BK7231N
	os_pin_write(SW_RED_LED_P20, PIN_LOW);
	os_pin_write(SW_GREEN_LED_P21, PIN_LOW);
#endif
	return;
}

void acw_green_led_open(void)
{
	if (OS_FALSE ==  gs_led_io_inited)
	{
		return;
	}

#ifdef OS_USING_LED
	os_pin_write(led_table[LED_RED].pin, PIN_HIGH);	
	os_pin_write(led_table[LED_GREEN].pin, PIN_LOW);
#endif
#ifdef BOARD_BK7231N
	os_pin_write(SW_RED_LED_P20, PIN_LOW);
	os_pin_write(SW_GREEN_LED_P21, PIN_HIGH);
#endif
	return;	
}

void acw_all_led_close(void)
{
	if (OS_FALSE ==  gs_led_io_inited)
	{
		return;
	}

#ifdef OS_USING_LED	
	os_pin_write(led_table[LED_RED].pin, PIN_HIGH);
	os_pin_write(led_table[LED_GREEN].pin, PIN_HIGH);
#endif

#ifdef BOARD_BK7231N
	os_pin_write(SW_GREEN_LED_P21, PIN_LOW);
	os_pin_write(SW_RED_LED_P20, PIN_LOW);
#endif
	return;
}
