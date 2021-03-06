/**
 ****************************************************************************************
 *
 * @file kernel_config.h
 *
 * @brief This file contains all the constant that can be changed in order to
 * tailor the kernel.
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef _KERNEL_CONFIG_H_
#define _KERNEL_CONFIG_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"       // stack configuration

/**
 ****************************************************************************************
 * @addtogroup KERNEL KERNEL
 * @ingroup ROOT
 * @brief The Kernel module.
 *
 * The Kernel is responsible for providing essential OS features like time management,
 * inter-task communication, task management and message handling and administration.
 *
 * @{
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup CFG Settings and Configuration
 * @ingroup KERNEL
 * @brief Kernel Configuration
 *
 * @{
 ****************************************************************************************
 */

/*
 * CONSTANT DEFINITIONS
 ****************************************************************************************
 */
#define KERNEL_MEM_RW       0
#define KERNEL_MEM_LINUX    0
#define KERNEL_MEM_LIBC     0

#define KERNEL_FULL         1
#define KERNEL_SEND_ONLY    0

/// @} CFG

#endif // _KERNEL_CONFIG_H_
