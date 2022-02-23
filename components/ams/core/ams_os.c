#include <string.h>
#include "board.h"
#include "core/ams_core.h"
#if defined(AMS_USING_PLATFORM) && defined(NET_USING_MOLINK)
#include <mo_api.h>
#endif

/* dock with os */
static ams_thd_ctrl_entry_t g_ams_thread[AMS_THD_MAX] = {
        {NULL, "ams_main_thd", AMS_MAIN_THD_STACK_SIZE, AMS_MAIN_THD_PRIOIRTY}, 
        {NULL, "Ams-run-thd", AMS_RUN_THD_STACK_SIZE, AMS_RUN_THD_PRIOIRTY},
};

static ams_env_set_t g_ams_env[AMS_APP_TYPE_MAX] = {
        {micropy_file_exit, micropy_file_state, mpy_deinit},
        {NULL, NULL},
 };

/* create and run thread */
int ams_startup_thread(AMS_THREAD_TYPE_T thd_type, void (*entry)(void *arg), void *arg)
{
    (void)ams_stop_thread(thd_type);
    g_ams_thread[thd_type].thread = os_task_create(g_ams_thread[thd_type].name, 
                                            entry, 
                                            arg,
                                            g_ams_thread[thd_type].stack_size, 
                                            g_ams_thread[thd_type].priority);
    if (g_ams_thread[thd_type].thread == NULL) 
    {
        ams_err("Create thread[%d] failed.", thd_type);
        return AMS_ERROR;
    } 
    else 
    {
        os_task_startup(g_ams_thread[thd_type].thread);
    }

    return AMS_OK;
}

int ams_stop_thread(AMS_THREAD_TYPE_T thd_type)
{
    if (g_ams_thread[thd_type].thread != NULL)
    {
        if (os_task_destroy(g_ams_thread[thd_type].thread) != AMS_OK)
        {
            ams_err("When stop thread[%d], destroy the ex-thread failed.", thd_type);
            return AMS_ERROR;
        }
        g_ams_thread[thd_type].thread = NULL;
        if (g_ams_env[AMS_APP_TYPE_CHOICE].app_state != NULL)
        {
            if (g_ams_env[AMS_APP_TYPE_CHOICE].app_state()) 
            {   /* !0: destroy when app running, need to active deinit */
                g_ams_env[AMS_APP_TYPE_CHOICE].deinit();
            }
        }
        if(g_ams_env[AMS_APP_TYPE_CHOICE].app_unlock != NULL)
        {
            g_ams_env[AMS_APP_TYPE_CHOICE].app_unlock();
        }
    }

    return AMS_OK;
}

static int ams_get_module_info_from_conf(char *str_buff, const char *conf_str, int info_len)
{
    if (strlen(conf_str) != info_len)
    {
        ams_log("Input a wrong imei[%s], length[%d]", conf_str, strlen(conf_str));
        return AMS_ERROR;
    }

    strcpy(str_buff, conf_str);
    return AMS_OK;
}


int ams_get_imei(char *imei, int len)
{
    int ret = AMS_ERROR;
    
#if defined(AMS_USING_PLATFORM) && defined(NET_USING_MOLINK)
    /* get information from mo first, if failed, use the configure */
    ret = mo_get_imei(mo_get_default(), imei, len);
    if (ret == AMS_OK)
    {
        ams_log("Get imei[%s] info success.", imei);
        return AMS_OK;
    }
#endif
    ret = ams_get_module_info_from_conf(imei, AMS_DEVICE_IMEI, AMS_IMEI_NAME_LEN);
    if (ret != AMS_OK)
    {
        ams_err("Get imei[%s] info failed.", AMS_DEVICE_IMEI);
    }

    return ret;
}

int ams_get_imsi(char *imsi, int len)
{
    int ret = AMS_ERROR;
#if defined(AMS_USING_PLATFORM) && defined(NET_USING_MOLINK)
    /* get information from mo first, if failed, use the configure */
    ret = mo_get_imsi(mo_get_default(), imsi, len);
    if (ret == AMS_OK)
    {
        ams_log("Get imsi[%s] info success.", imsi);
        return AMS_OK;
    }
#endif

    ret = ams_get_module_info_from_conf(imsi, AMS_DEVICE_IMSI, AMS_IMSI_NAME_LEN);
    if (ret != AMS_OK)
    {
        ams_err("Get imsi[%s] info failed.", AMS_DEVICE_IMSI);
    }

    return ret;
}

int ams_get_iccid(char *iccid, int len)
{
    int ret = AMS_ERROR;
#if defined(AMS_USING_PLATFORM) && defined(NET_USING_MOLINK)
    /* get information from mo first, if failed, use the configure */
    ret = mo_get_iccid(mo_get_default(), iccid, len);
    if (ret == AMS_OK)
    {
        ams_log("Get iccid[%s] info success.", iccid);
        return AMS_OK;
    }
#endif
    ret = ams_get_module_info_from_conf(iccid, AMS_DEVICE_ICCID, AMS_ICCID_NAME_LEN);
    if (ret != AMS_OK)
    {
        ams_err("Get iccid[%s] info failed.", AMS_DEVICE_ICCID);
    }

    return ret;

}

void ams_reset_board(void)
{
    HAL_NVIC_SystemReset();
    return;
}

