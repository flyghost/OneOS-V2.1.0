
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


#include "modules/iotjs_module_i2c.h"

struct iotjs_i2c_platform_data_s {
  iotjs_string_t device;
  bool device_avail;
  int device_fd;
  uint8_t addr;
};

void iotjs_i2c_create_platform_data(iotjs_i2c_t* i2c) {
  i2c->platform_data = IOTJS_ALLOC(iotjs_i2c_platform_data_t);
  i2c->platform_data->device_fd = -1;
  i2c->platform_data->device_avail = false;
}

void iotjs_i2c_destroy_platform_data(iotjs_i2c_platform_data_t* pdata) {
  if (pdata->device_avail)
    iotjs_string_destroy(&pdata->device);
  IOTJS_RELEASE(pdata);
}

jerry_value_t iotjs_i2c_set_platform_config(iotjs_i2c_t* i2c,
                                            const jerry_value_t jconfig) {
  iotjs_i2c_platform_data_t* platform_data = i2c->platform_data;

  JS_GET_REQUIRED_CONF_VALUE(jconfig, platform_data->device,
                             IOTJS_MAGIC_STRING_DEVICE, string);
  i2c->platform_data->device_avail = true;

  return jerry_create_undefined();
}


#define I2C_METHOD_HEADER(arg)                                   \
  iotjs_i2c_platform_data_t* platform_data = arg->platform_data; \
  IOTJS_ASSERT(platform_data);                                   \
  if (platform_data->device_fd < 0) {                            \
    DLOG("%s: I2C is not opened", __func__);                     \
    return false;                                                \
  }

bool iotjs_i2c_open(iotjs_i2c_t* i2c) {
  iotjs_i2c_platform_data_t* platform_data = i2c->platform_data;

  platform_data->device_fd =
      open(iotjs_string_data(&platform_data->device), O_RDWR);

  if (platform_data->device_fd == -1) {
    DLOG("%s : cannot open", __func__);
    return false;
  }

  platform_data->addr = i2c->address;

  return true;
}

bool iotjs_i2c_close(iotjs_i2c_t* i2c) {
  I2C_METHOD_HEADER(i2c);

  if (close(platform_data->device_fd) < 0) {
    DLOG("%s : cannot close", __func__);
    return false;
  }

  platform_data->device_fd = -1;

  return true;
}

bool iotjs_i2c_write(iotjs_i2c_t* i2c) {
  I2C_METHOD_HEADER(i2c);

  uint8_t len = i2c->buf_len;
  char* data = i2c->buf_data;

  lseek(platform_data->device_fd, platform_data->addr, SEEK_SET);
  int ret = write(platform_data->device_fd, data, len);

  IOTJS_RELEASE(i2c->buf_data);

  return ret == len;
}

bool iotjs_i2c_read(iotjs_i2c_t* i2c) {
  I2C_METHOD_HEADER(i2c);

  uint8_t len = i2c->buf_len;
  i2c->buf_data = iotjs_buffer_allocate(len);

  lseek(platform_data->device_fd, platform_data->addr, SEEK_SET);
  return read(platform_data->device_fd, i2c->buf_data, len) == len;
}
