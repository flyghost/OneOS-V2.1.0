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
 * @file        mo_factory.c
 *
 * @brief       module link kit factory mode api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "mo_factory.h"
#include <inttypes.h>

#define MO_LOG_TAG "module.factory"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

/***************************************** Start to list of NB-IoT modules ********************************************/
#ifdef MOLINK_USING_M5310A
#include "m5310a.h"
#endif

#ifdef MOLINK_USING_M5311
#include "m5311.h"
#endif

#ifdef MOLINK_USING_BC95
#include "bc95.h"
#endif

#ifdef MOLINK_USING_BC28
#include "bc28.h"
#endif

#ifdef MOLINK_USING_BC26
#include "bc26.h"
#endif

#ifdef MOLINK_USING_BC20
#include "bc20.h"
#endif

#ifdef MOLINK_USING_SIM7020
#include "sim7020.h"
#endif

#ifdef MOLINK_USING_SIM7070X
#include "sim7070x.h"
#endif

#ifdef MOLINK_USING_N21
#include "n21.h"
#endif

#ifdef MOLINK_USING_MB26
#include "mb26.h"
#endif
#ifdef MOLINK_USING_GM120
#include "gm120.h"
#endif

#ifdef MOLINK_USING_E7025
#include "e7025.h"
#endif

#ifdef MOLINK_USING_ME3616
#include "me3616.h"
#endif
/***************************************** End of NB-IoT modules list *************************************************/


/*************************************** Start to list of 4G cat1 modules *********************************************/
#ifdef MOLINK_USING_EC200X_600S
#include "ec200x_600s.h"
#endif

#ifdef MOLINK_USING_ML302
#include "ml302.h"
#endif

#ifdef MOLINK_USING_GM190
#include "gm190.h"
#endif

#ifdef MOLINK_USING_A7600X
#include "a7600x.h"
#endif

#ifdef MOLINK_USING_A7670X
#include "a7670x.h"
#endif

#ifdef MOLINK_USING_L610
#include "l610.h"
#endif

#ifdef MOLINK_USING_AIR723UG
#include "air723ug.h"
#endif

#ifdef MOLINK_USING_CLM920RV3
#include "clm920rv3.h"
#endif
#ifdef MOLINK_USING_N58
#include "n58.h"
#endif

#ifdef MOLINK_USING_AIR720UH
#include "air720uh.h"
#endif
/**************************************** End of 4G cat1 modules list *************************************************/


/*************************************** Start to list of 4G cat4 modules *********************************************/

#ifdef MOLINK_USING_GM510
#include "gm510.h"
#endif

#ifdef MOLINK_USING_EC20
#include "ec20.h"
#endif

#ifdef MOLINK_USING_SIM7600CE
#include "sim7600ce.h"
#endif

#ifdef MOLINK_USING_ME3630_W
#include "me3630_w.h"
#endif

/**************************************** End of 4G cat4 modules list *************************************************/

/*************************************** Start to list of 5G modules **************************************************/
#ifdef MOLINK_USING_RG500Q
#include "rg500q.h"
#endif
/**************************************** End of 5G modules list ******************************************************/

/*************************************** Start to list of wifi modules ************************************************/
#ifdef MOLINK_USING_ESP8266
#include "esp8266.h"
#endif
#ifdef MOLINK_USING_ESP32
#include "esp32.h"
#endif
/***************************************** End of wifi modules list ***************************************************/


static const mo_create_fn gs_mo_create_t[] = {
/* Type of NB-IoT modules */
#ifdef MOLINK_USING_M5310A
    [MODULE_TYPE_M5310A] = module_m5310a_create,
#endif
#ifdef MOLINK_USING_M5311
    [MODULE_TYPE_M5311] = module_m5311_create,
#endif
#ifdef MOLINK_USING_BC95
    [MODULE_TYPE_BC95] = module_bc95_create,
#endif
#ifdef MOLINK_USING_BC28
    [MODULE_TYPE_BC28] = module_bc28_create,
#endif
#ifdef MOLINK_USING_BC26
    [MODULE_TYPE_BC26] = module_bc26_create,
#endif
#ifdef MOLINK_USING_BC20
    [MODULE_TYPE_BC20] = module_bc20_create,
#endif
#ifdef MOLINK_USING_SIM7020
    [MODULE_TYPE_SIM7020] = module_sim7020_create,
#endif
#ifdef MOLINK_USING_SIM7070X
    [MODULE_TYPE_SIM7070X] = module_sim7070x_create,
#endif
#ifdef MOLINK_USING_N21
    [MODULE_TYPE_N21] = module_n21_create,
#endif
#ifdef MOLINK_USING_MB26
    [MODULE_TYPE_MB26] = module_mb26_create,
#endif
#ifdef MOLINK_USING_GM120
    [MODULE_TYPE_GM120] = module_gm120_create,
#endif
#ifdef MOLINK_USING_E7025
    [MODULE_TYPE_E7025] = module_e7025_create,
#endif
#ifdef MOLINK_USING_ME3616
    [MODULE_TYPE_ME3616] = module_me3616_create,
#endif

/* Type of 4G cat1 modules */
#ifdef MOLINK_USING_EC200X_600S
    [MODULE_TYPE_EC200X_600S] = module_ec200x_600s_create,
#endif
#ifdef MOLINK_USING_ML302
    [MODULE_TYPE_ML302] = module_ml302_create,
#endif
#ifdef MOLINK_USING_GM190
    [MODULE_TYPE_GM190] = module_gm190_create,
#endif
#ifdef MOLINK_USING_A7600X
    [MODULE_TYPE_A7600X] = module_a7600x_create,
#endif
#ifdef MOLINK_USING_A7670X
    [MODULE_TYPE_A7670X] = module_a7670x_create,
#endif
#ifdef MOLINK_USING_L610
    [MODULE_TYPE_L610] = module_l610_create,
#endif
#ifdef MOLINK_USING_AIR723UG
    [MODULE_TYPE_AIR723UG] = module_air723ug_create,
#endif
#ifdef MOLINK_USING_CLM920RV3
    [MODULE_TYPE_CLM920RV3] = module_clm920rv3_create,
#endif
#ifdef MOLINK_USING_N58
    [MODULE_TYPE_N58] = module_n58_create,
#endif
#ifdef MOLINK_USING_AIR720UH
    [MODULE_TYPE_AIR720UH] = module_air720uh_create,
#endif

/* Type of 4G cat4 modules */
#ifdef MOLINK_USING_GM510
    [MODULE_TYPE_GM510] = module_gm510_create,
#endif
#ifdef MOLINK_USING_EC20
    [MODULE_TYPE_EC20] = module_ec20_create,
#endif
#ifdef MOLINK_USING_SIM7600CE
    [MODULE_TYPE_SIM7600CE] = module_sim7600ce_create,
#endif
#ifdef MOLINK_USING_ME3630_W
    [MODULE_TYPE_ME3630_W] = module_me3630_w_create,
#endif

/* Type of 5G modules */
#ifdef MOLINK_USING_RG500Q
    [MODULE_TYPE_RG500Q] = module_rg500q_create,
#endif

/* Type of wifi modules */
#ifdef MOLINK_USING_ESP8266
    [MODULE_TYPE_ESP8266] = module_esp8266_create,
#endif
#ifdef MOLINK_USING_ESP32
    [MODULE_TYPE_ESP32] = module_esp32_create,
#endif

    [MODULE_TYPE_MAX] = OS_NULL,
};

static const mo_destory_fn gs_mo_destroy_t[] = {
/* Type of NB-IoT modules */
#ifdef MOLINK_USING_M5310A
    [MODULE_TYPE_M5310A] = module_m5310a_destroy,
#endif
#ifdef MOLINK_USING_M5311
    [MODULE_TYPE_M5311] = module_m5311_destroy,
#endif
#ifdef MOLINK_USING_BC95
    [MODULE_TYPE_BC95] = module_bc95_destroy,
#endif
#ifdef MOLINK_USING_BC28
    [MODULE_TYPE_BC28] = module_bc28_destroy,
#endif
#ifdef MOLINK_USING_BC26
    [MODULE_TYPE_BC26] = module_bc26_destroy,
#endif
#ifdef MOLINK_USING_BC20
    [MODULE_TYPE_BC20] = module_bc20_destroy,
#endif
#ifdef MOLINK_USING_SIM7020
    [MODULE_TYPE_SIM7020] = module_sim7020_destroy,
#endif
#ifdef MOLINK_USING_SIM7070X
    [MODULE_TYPE_SIM7070X] = module_sim7070x_destroy,
#endif
#ifdef MOLINK_USING_N21
    [MODULE_TYPE_N21] = module_n21_destroy,
#endif
#ifdef MOLINK_USING_MB26
    [MODULE_TYPE_MB26] = module_mb26_destroy,
#endif
#ifdef MOLINK_USING_GM120
    [MODULE_TYPE_GM120] = module_gm120_destroy,
#endif
#ifdef MOLINK_USING_E7025
    [MODULE_TYPE_E7025] = module_e7025_destroy,
#endif
#ifdef MOLINK_USING_ME3616
    [MODULE_TYPE_ME3616] = module_me3616_destroy,
#endif

/* Type of 4G cat1 modules */
#ifdef MOLINK_USING_EC200X_600S
    [MODULE_TYPE_EC200X_600S] = module_ec200x_600s_destroy,
#endif
#ifdef MOLINK_USING_ML302
    [MODULE_TYPE_ML302] = module_ml302_destroy,
#endif
#ifdef MOLINK_USING_GM190
    [MODULE_TYPE_GM190] = module_gm190_destroy,
#endif
#ifdef MOLINK_USING_A7600X
    [MODULE_TYPE_A7600X] = module_a7600x_destroy,
#endif
#ifdef MOLINK_USING_A7670X
    [MODULE_TYPE_A7670X] = module_a7670x_destroy,
#endif
#ifdef MOLINK_USING_L610
    [MODULE_TYPE_L610] = module_l610_destroy,
#endif
#ifdef MOLINK_USING_AIR723UG
    [MODULE_TYPE_AIR723UG] = module_air723ug_destroy,
#endif
#ifdef MOLINK_USING_CLM920RV3
    [MODULE_TYPE_CLM920RV3] = module_clm920rv3_destroy,
#endif
#ifdef MOLINK_USING_N58
    [MODULE_TYPE_N58] = module_n58_destroy,
#endif
#ifdef MOLINK_USING_AIR720UH
    [MODULE_TYPE_AIR720UH] = module_air720uh_destroy,
#endif

/* Type of 4G cat4 modules */
#ifdef MOLINK_USING_GM510
    [MODULE_TYPE_GM510] = module_gm510_destroy,
#endif
#ifdef MOLINK_USING_EC20
    [MODULE_TYPE_EC20] = module_ec20_destroy,
#endif
#ifdef MOLINK_USING_SIM7600CE
    [MODULE_TYPE_SIM7600CE] = module_sim7600ce_destroy,
#endif
#ifdef MOLINK_USING_ME3630_W
    [MODULE_TYPE_ME3630_W] = module_me3630_w_destroy,
#endif

/* Type of 5G modules */
#ifdef MOLINK_USING_RG500Q
    [MODULE_TYPE_RG500Q] = OS_NULL,
#endif

/* Type of wifi modules */
#ifdef MOLINK_USING_ESP8266
    [MODULE_TYPE_ESP8266] = module_esp8266_destroy,
#endif
#ifdef MOLINK_USING_ESP32
    [MODULE_TYPE_ESP32] = module_esp32_destroy,
#endif

    [MODULE_TYPE_MAX] = OS_NULL,
};

/**
 ***********************************************************************************************************************
 * @brief           Create an instance of a molink module object
 *
 * @param[in]       name            The molink module instance name
 * @param[in]       type            The type of molink module object. @ref mo_type_t
 * @param[in]       device          The device used by molink module instance at parser
 * @param[in]       recv_len        The receive buffer length of at parser
 *
 * @return          On success, return a molink module instance descriptor;
 *                  On error, OS_NULL is returned.
 ***********************************************************************************************************************
 */
mo_object_t *mo_create(const char *name, mo_type_t type, void *parser_config)
{
    OS_ASSERT(OS_NULL != name);

    if (type <= MODULE_TYPE_NULL || type >= MODULE_TYPE_MAX)
    {
        ERROR("Failed to create module object, module type error");
        return OS_NULL;
    }

    if (OS_NULL == gs_mo_create_t[type])
    {
        ERROR("The system did not find the create function for the module %s", name);
        return OS_NULL;
    }

    mo_object_t *module = gs_mo_create_t[type](name, parser_config);
    if (OS_NULL != module)
    {
        module->type = type;
    }

    return module;
}

/**
 ***********************************************************************************************************************
 * @brief           Destroy an instance of a molink module object
 *
 * @param[in]       self            The molink module instance descriptor
 * @param[in]       type            The type of molink module object. @ref mo_type_t
 *
 * @return          Returns the result of the operation
 * @retval          OS_ERROR        Destroy failed
 * @retval          OS_EOK          Destroy successfully
 ***********************************************************************************************************************
 */
os_err_t mo_destroy(mo_object_t *self, mo_type_t type)
{
    OS_ASSERT(OS_NULL != self);

    if (type <= MODULE_TYPE_NULL || type >= MODULE_TYPE_MAX)
    {
        ERROR("Failed to destroy module object, module type error");
        return OS_ERROR;
    }

    if (OS_NULL == gs_mo_destroy_t[type])
    {
        ERROR("The system did not find the destroy function for the module %s", self->name);
        return OS_ERROR;
    }

    return gs_mo_destroy_t[type](self);
}
