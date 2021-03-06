/**
 ****************************************************************************************
 *
 * @file rwble.h
 *
 * @brief Entry points of the BLE software
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 ****************************************************************************************
 */

#ifndef RWBLE_H_
#define RWBLE_H_

/**
 ****************************************************************************************
 * @addtogroup ROOT
 * @brief Entry points of the BLE stack
 *
 * This module contains the primitives that allow an application accessing and running the
 * BLE protocol stack
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>
#include <stdbool.h>
#include "rwip_config.h"
#include "ble_compiler.h"
#include "common_error.h"

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialize the BLE stack.
 ****************************************************************************************
 */
void rwble_init(void);


/**
 ****************************************************************************************
 * @brief Reset the BLE stack.
 ****************************************************************************************
 */
void rwble_reset(void);

#if (SYSTEM_SLEEP)
/**
 ****************************************************************************************
 * @brief Return true if no BLE activity on going
 *
 ****************************************************************************************
 */
bool rwble_sleep_check(void);
#endif

/**
 ****************************************************************************************
 * @brief Return true if no BLE activity on going
 *
 ****************************************************************************************
 */
bool rwble_activity_ongoing_check(void);

/**
 ****************************************************************************************
 * @brief Gives FW/HW versions of RW-BLE stack.
 *
 ****************************************************************************************
 */
void rwble_version(uint8_t *fw_version, uint8_t *hw_version);

/**
 ****************************************************************************************
 * @brief RWBLE interrupt service routine
 *
 * This function is the interrupt service handler of RWBLE.
 *
 ****************************************************************************************
 */
__BLEIRQ void rwble_isr(void);

/// @} RWBLE

#endif // RWBLE_H_
