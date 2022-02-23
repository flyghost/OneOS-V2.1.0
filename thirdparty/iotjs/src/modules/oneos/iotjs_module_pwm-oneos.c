
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iotjs_systemio-linux.h"
#include "modules/iotjs_module_pwm.h"

#include "device.h"
#include "pwm.h"

struct iotjs_pwm_platform_data_s {
  iotjs_string_t device;
  bool device_avail;
  os_device_t *pwm_dev;
  struct os_pwm_configuration config;
};

void iotjs_pwm_create_platform_data(iotjs_pwm_t* pwm) {
  pwm->platform_data = IOTJS_ALLOC(iotjs_pwm_platform_data_t);
  pwm->platform_data->pwm_dev = NULL;
  pwm->platform_data->device_avail = false;
  pwm->platform_data->config.channel = 1;
  pwm->platform_data->config.period = 1e6;
  pwm->platform_data->config.pulse = 1e5;
}

void iotjs_pwm_destroy_platform_data(iotjs_pwm_platform_data_t* pdata) {
  if (pdata->device_avail)
    iotjs_string_destroy(&pdata->device);
  IOTJS_RELEASE(pdata);
}

jerry_value_t iotjs_pwm_set_platform_config(iotjs_pwm_t* pwm,
                                            const jerry_value_t jconfig) {
  struct iotjs_pwm_platform_data_s* platform_data = pwm->platform_data;

  JS_GET_REQUIRED_CONF_VALUE(jconfig, platform_data->device,
                             IOTJS_MAGIC_STRING_DEVICE, string);
  pwm->platform_data->device_avail = true;
  return jerry_create_undefined();
}

static const char * remove_dev(const iotjs_string_t* str) {
  int i;
  for(i = str->size - 1; i >= 0; i--) {
    if (str->data[i] == '/')
      break;
  }
  return &str->data[i + 1];
}

bool iotjs_pwm_open(iotjs_pwm_t* pwm) {
  iotjs_pwm_platform_data_t* platform_data = pwm->platform_data;

  platform_data->config.period = pwm->period * 1e9;
  platform_data->config.pulse = pwm->duty_cycle * platform_data->config.period;

  platform_data->pwm_dev = os_device_find(remove_dev(&platform_data->device));
  if (NULL == platform_data->pwm_dev)
    return false;

  // Set options.
  if (pwm->period >= 0) {
    if (!iotjs_pwm_set_period(pwm)) {
      return false;
    }
    if (pwm->duty_cycle >= 0) {
      if (!iotjs_pwm_set_dutycycle(pwm)) {
        return false;
      }
    }
  }

  return true;
}

bool iotjs_pwm_set_period(iotjs_pwm_t* pwm) {
  iotjs_pwm_platform_data_t* platform_data = pwm->platform_data;

  platform_data->config.period = pwm->period * 1e9;
  platform_data->config.pulse = pwm->duty_cycle * platform_data->config.period;

  os_err_t err = os_device_control(platform_data->pwm_dev,
                      OS_PWM_CMD_SET_PERIOD,
                      &platform_data->config);
  if (OS_EOK != err)
    return false;
  err = os_device_control(platform_data->pwm_dev,
                      OS_PWM_CMD_SET_PULSE,
                      &platform_data->config);
  if (OS_EOK != err)
    return false;

  return true;
}


bool iotjs_pwm_set_dutycycle(iotjs_pwm_t* pwm) {
  iotjs_pwm_platform_data_t* platform_data = pwm->platform_data;

  platform_data->config.period = pwm->period * 1e9;
  platform_data->config.pulse = pwm->duty_cycle * platform_data->config.period;

  os_err_t err = os_device_control(platform_data->pwm_dev,
                      OS_PWM_CMD_SET_PERIOD,
                      &platform_data->config);
  if (OS_EOK != err)
    return false;
  err = os_device_control(platform_data->pwm_dev,
                      OS_PWM_CMD_SET_PULSE,
                      &platform_data->config);
  if (OS_EOK != err)
    return false;

  return true;;
}


bool iotjs_pwm_set_enable(iotjs_pwm_t* pwm) {
  iotjs_pwm_platform_data_t* platform_data = pwm->platform_data;
  os_err_t err;

  if (pwm->enable) {
    err = os_device_control(platform_data->pwm_dev,
                      OS_PWM_CMD_ENABLE,
                      &platform_data->config);
  } else {
    err = os_device_control(platform_data->pwm_dev,
                      OS_PWM_CMD_DISABLE,
                      &platform_data->config);
  }

  if (OS_EOK == err)
    return true;
  else
    return false;
}


bool iotjs_pwm_close(iotjs_pwm_t* pwm) {
  iotjs_pwm_platform_data_t* platform_data = pwm->platform_data;

  platform_data->pwm_dev = NULL;

  return true;
}
