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
 * @file        ingenic_gpio.h
 *
 * @brief       This file provides gpio functions.
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-17   OneOS Team      First Version
 ***********************************************************************************************************************
 */


#ifndef INGENIC_GPIO_H_
#define INGENIC_GPIO_H_

#include <stdint.h>
#include <t31.h>

#define JZGPIO_GROUP_OFFSET     (0x1000)

#define GPIO_PA(n) 	(0*32 + n)
#define GPIO_PB(n) 	(1*32 + n)
#define GPIO_PC(n) 	(2*32 + n)
#define GPIO_PD(n) 	(3*32 + n)

enum gpio_function {
    GPIO_FUNC_0     = 0x00,  //0000, GPIO as function 0 / device 0
    GPIO_FUNC_1     = 0x01,  //0001, GPIO as function 1 / device 1
    GPIO_FUNC_2     = 0x02,  //0010, GPIO as function 2 / device 2
    GPIO_FUNC_3     = 0x03,  //0011, GPIO as function 3 / device 3
    GPIO_OUTPUT0    = 0x04,  //0100, GPIO output low  level
    GPIO_OUTPUT1    = 0x05,  //0101, GPIO output high level
    GPIO_INPUT	= 0x06,	 //0110, GPIO as input
    GPIO_RISE_EDGE  = 0x0b,	//1011, GPIO as rise edge interrupt
    GPIO_PULL       = 0x10,	//10000, GPIO set pull
};

enum gpio_port {
    GPIO_PORT_A,
    GPIO_PORT_B,
    GPIO_PORT_C,
    GPIO_PORT_D,
    /* this must be last */
    GPIO_NR_PORTS,
};

struct jz_gpio_func_def {
    int port;
    int func;
    unsigned long pins;
};

/*************************************************************************
 * GPIO (General-Purpose I/O Ports)
 *************************************************************************/
#define MAX_GPIO_NUM	192

#define PXPUEN		0x110   /* Port Pull-up status Register */
#define PXPUENS		0x114   /* Port Pull-up status Set Register */
#define PXPUENC		0x118   /* Port Pull-up status Clear Register */
#define PXPDEN		0x120   /* Port Pull-down status Register */
#define PXPDENS		0x124   /* Port Pull-down status Set Register */
#define PXPDENC		0x128   /* Port Pull-down status Clear Register */
#define PXPDSL		0x130   /* Port Driver-strength low  Register */
#define PXPDSLS		0x134   /* Port Driver-strength low Set Register */
#define PXPDSLC		0x138   /* Port Driver-strength low Clear Register */
#define PXPDSH		0x140   /* Port Driver-strength high  Register */
#define PXPDSHS		0x144   /* Port Driver-strength high Set Register */
#define PXPDSHC		0x148   /* Port Driver-strength high Clear Register */

#define PXPIN		0x00   /* PIN Level Register */
#define PXINT		0x10   /* Port Interrupt Register */
#define PXINTS		0x14   /* Port Interrupt Set Register */
#define PXINTC		0x18   /* Port Interrupt Clear Register */
#define PXMSK		0x20   /* Port Interrupt Mask Reg */
#define PXMSKS		0x24   /* Port Interrupt Mask Set Reg */
#define PXMSKC		0x28   /* Port Interrupt Mask Clear Reg */
#define PXPAT1		0x30   /* Port Pattern 1 Set Reg. */
#define PXPAT1S		0x34   /* Port Pattern 1 Set Reg. */
#define PXPAT1C		0x38   /* Port Pattern 1 Clear Reg. */
#define PXPAT0		0x40   /* Port Pattern 0 Register */
#define PXPAT0S		0x44   /* Port Pattern 0 Set Register */
#define PXPAT0C		0x48   /* Port Pattern 0 Clear Register */
#define PXFLG		0x50   /* Port Flag Register */
#define PXFLGC		0x58   /* Port Flag clear Register */
#define PXPE		0x70   /* Port Pull Disable Register */
#define PXPES		0x74   /* Port Pull Disable Set Register */
#define PXPEC		0x78   /* Port Pull Disable Clear Register */

#define GPIO_PXPIN(n)	(GPIO_BASE + (PXPIN + (n)*0x100)) /* PIN Level Register */
#define GPIO_PXINT(n)	(GPIO_BASE + (PXINT + (n)*0x100)) /* Port Interrupt Register */
#define GPIO_PXINTS(n)	(GPIO_BASE + (PXINTS + (n)*0x100)) /* Port Interrupt Set Register */
#define GPIO_PXINTC(n)	(GPIO_BASE + (PXINTC + (n)*0x100)) /* Port Interrupt Clear Register */
#define GPIO_PXMSK(n)	(GPIO_BASE + (PXMSK + (n)*0x100)) /* Port Interrupt Mask Register */
#define GPIO_PXMSKS(n)	(GPIO_BASE + (PXMSKS + (n)*0x100)) /* Port Interrupt Mask Set Reg */
#define GPIO_PXMSKC(n)	(GPIO_BASE + (PXMSKC + (n)*0x100)) /* Port Interrupt Mask Clear Reg */
#define GPIO_PXPAT1(n)	(GPIO_BASE + (PXPAT1 + (n)*0x100)) /* Port Pattern 1 Register */
#define GPIO_PXPAT1S(n)	(GPIO_BASE + (PXPAT1S + (n)*0x100)) /* Port Pattern 1 Set Reg. */
#define GPIO_PXPAT1C(n)	(GPIO_BASE + (PXPAT1C + (n)*0x100)) /* Port Pattern 1 Clear Reg. */
#define GPIO_PXPAT0(n)	(GPIO_BASE + (PXPAT0 + (n)*0x100)) /* Port Pattern 0 Register */
#define GPIO_PXPAT0S(n)	(GPIO_BASE + (PXPAT0S + (n)*0x100)) /* Port Pattern 0 Set Register */
#define GPIO_PXPAT0C(n)	(GPIO_BASE + (PXPAT0C + (n)*0x100)) /* Port Pattern 0 Clear Register */
#define GPIO_PXFLG(n)	(GPIO_BASE + (PXFLG + (n)*0x100)) /* Port Flag Register */
#define GPIO_PXFLGC(n)	(GPIO_BASE + (PXFLGC + (n)*0x100)) /* Port Flag clear Register */
#define GPIO_PXPE(n)	(GPIO_BASE + (PXPE + (n)*0x100)) /* Port Pull Disable Register */
#define GPIO_PXPES(n)	(GPIO_BASE + (PXPES + (n)*0x100)) /* Port Pull Disable Set Register */
#define GPIO_PXPEC(n)	(GPIO_BASE + (PXPEC + (n)*0x100)) /* Port Pull Disable Clear Register */

enum gpio_pin {
    GPIO_Pin_0     = ((uint32_t)0x00000001),  /* Pin 0  selected */
    GPIO_Pin_1     = ((uint32_t)0x00000002),  /* Pin 1  selected */
    GPIO_Pin_2     = ((uint32_t)0x00000004),  /* Pin 2  selected */
    GPIO_Pin_3     = ((uint32_t)0x00000008),  /* Pin 3  selected */
    GPIO_Pin_4     = ((uint32_t)0x00000010),  /* Pin 4  selected */
    GPIO_Pin_5     = ((uint32_t)0x00000020),  /* Pin 5  selected */
    GPIO_Pin_6     = ((uint32_t)0x00000040),  /* Pin 6  selected */
    GPIO_Pin_7     = ((uint32_t)0x00000080),  /* Pin 7  selected */
    GPIO_Pin_8     = ((uint32_t)0x00000100),  /* Pin 8  selected */
    GPIO_Pin_9     = ((uint32_t)0x00000200),  /* Pin 9  selected */
    GPIO_Pin_10    = ((uint32_t)0x00000400),  /* Pin 10 selected */
    GPIO_Pin_11    = ((uint32_t)0x00000800),  /* Pin 11 selected */
    GPIO_Pin_12    = ((uint32_t)0x00001000),  /* Pin 12 selected */
    GPIO_Pin_13    = ((uint32_t)0x00002000),  /* Pin 13 selected */
    GPIO_Pin_14    = ((uint32_t)0x00004000),  /* Pin 14 selected */
    GPIO_Pin_15    = ((uint32_t)0x00008000),  /* Pin 15 selected */
    GPIO_Pin_16    = ((uint32_t)0x00010000),  /* Pin 16 selected */
    GPIO_Pin_17    = ((uint32_t)0x00020000),  /* Pin 17 selected */
    GPIO_Pin_18    = ((uint32_t)0x00040000),  /* Pin 18 selected */
    GPIO_Pin_19    = ((uint32_t)0x00080000),  /* Pin 19 selected */
    GPIO_Pin_20    = ((uint32_t)0x00100000),  /* Pin 20 selected */
    GPIO_Pin_21    = ((uint32_t)0x00200000),  /* Pin 21 selected */
    GPIO_Pin_22    = ((uint32_t)0x00400000),  /* Pin 22 selected */
    GPIO_Pin_23    = ((uint32_t)0x00800000),  /* Pin 23 selected */
    GPIO_Pin_24    = ((uint32_t)0x01000000),  /* Pin 24 selected */
    GPIO_Pin_25    = ((uint32_t)0x02000000),  /* Pin 25 selected */
    GPIO_Pin_26    = ((uint32_t)0x04000000),  /* Pin 26 selected */
    GPIO_Pin_27    = ((uint32_t)0x08000000),  /* Pin 27 selected */
    GPIO_Pin_28    = ((uint32_t)0x10000000),  /* Pin 28 selected */
    GPIO_Pin_29    = ((uint32_t)0x20000000),  /* Pin 29 selected */
    GPIO_Pin_30    = ((uint32_t)0x40000000),  /* Pin 30 selected */
    GPIO_Pin_31    = ((uint32_t)0x80000000),  /* Pin 31 selected */

    GPIO_Pin_All   = ((uint32_t)0xFFFFFFFF),  /* All pins selected */
};

void gpio_set_func(enum gpio_port n, enum gpio_function func, unsigned int pins);

#endif

