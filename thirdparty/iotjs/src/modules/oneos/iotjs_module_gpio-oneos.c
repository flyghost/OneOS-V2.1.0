
#include "modules/iotjs_module_gpio.h"
#include "pin.h"

struct iotjs_gpio_platform_data_s {
  uint32_t pin;
#ifdef IOTJS_GPIO_ENABLE_IRQ
  bool attach_irq;
#endif
};

const uint32_t INVALID_PIN = (uint32_t)(-1);

const uint32_t gpioDirection[] = {
  PIN_MODE_INPUT, // kGpioDirectionIn
  PIN_MODE_OUTPUT, // kGpioDirectionOut
  0
};

const uint32_t gpioMode[] = {
  0, // kGpioModeNone
  PIN_MODE_INPUT_PULLUP, // kGpioModePullup
  PIN_MODE_INPUT_PULLDOWN, // kGpioModePulldown
  0, // kGpioModeFloat
  0, // kGpioModePushpull
  PIN_MODE_OUTPUT_OD, // kGpioModeOpendrain
  0
};

const uint32_t gpioEdge[] = {
  0, // kGpioEdgeNone
  PIN_IRQ_MODE_RISING, // kGpioEdgeRising
  PIN_IRQ_MODE_FALLING, // kGpioEdgeFalling
  PIN_IRQ_MODE_RISING_FALLING, // kGpioEdgeBoth
  0
};

#ifdef IOTJS_GPIO_ENABLE_IRQ
static void iotjs_gpio_irq_handler(void* arg);
#endif

#ifdef IOTJS_GPIO_ENABLE_IRQ
static bool _is_avaiable_edge(iotjs_gpio_t* gpio) {
  return (gpio->edge > kGpioEdgeNone &&
      gpio->edge < __kGpioEdgeMax);
}

static void _gpio_irq_detach(iotjs_gpio_platform_data_t* platform_data) {
  jerry_port_log(0, "_gpio_irq_detach %d\r\n", platform_data->pin);
  os_pin_irq_enable(platform_data->pin, PIN_IRQ_DISABLE);
  os_pin_detach_irq(platform_data->pin);
  platform_data->attach_irq = false;
}

static void _gpio_irq_attach(iotjs_gpio_t* gpio) {
  jerry_port_log(0, "_gpio_irq_attach %d %p\r\n", gpio->pin, gpio);
  os_pin_attach_irq(gpio->pin, gpioEdge[gpio->edge], iotjs_gpio_irq_handler, gpio);
  os_pin_irq_enable(gpio->pin, PIN_IRQ_ENABLE);
  gpio->platform_data->attach_irq = true;
}
#endif

static void _gpio_deinit(iotjs_gpio_t* gpio) {
#ifdef IOTJS_GPIO_ENABLE_IRQ
  if (gpio->platform_data->attach_irq) {
    _gpio_irq_detach(gpio->platform_data);
  }
#endif
}

static bool _gpio_init(iotjs_gpio_t* gpio) {

  // Set pin direction and mode
  uint32_t mode = gpioDirection[gpio->direction] | gpioMode[gpio->mode];

  os_pin_mode(gpio->pin, mode);

#ifdef IOTJS_GPIO_ENABLE_IRQ
  if (_is_avaiable_edge(gpio) &&
      gpio->direction == kGpioDirectionIn) {
    _gpio_irq_attach(gpio);
  } else if (gpio->platform_data->attach_irq) {
    _gpio_irq_detach(gpio->platform_data);
  }
#endif

  gpio->platform_data->pin = gpio->pin;

  return true;
}

#ifdef IOTJS_GPIO_ENABLE_IRQ
static void aysnc_irq_handler(void* param) {
  jerry_port_log(0, "aysnc_irq_handler %p\r\n", param);
  iotjs_gpio_t* gpio = (iotjs_gpio_t*)param;
  jerry_value_t jgpio = gpio->jobject;
  jerry_value_t jonChange = iotjs_jval_get_property(jgpio, "emit");
  IOTJS_ASSERT(jerry_value_is_function(jonChange));

  jerry_value_t type = jerry_create_string((const jerry_char_t*)("change"));
  jerry_value_t jargs[] = { type };

  jerry_value_t jres = jerry_call_function(jonChange, jgpio, jargs, 1);
  IOTJS_ASSERT(!jerry_value_is_error(jres));

  jerry_release_value(type);
  jerry_release_value(jres);
  jerry_release_value(jonChange);
}

static void iotjs_gpio_irq_handler(void* arg) {
  aysnc_irq_handler(arg);
}
#endif

void iotjs_gpio_create_platform_data(iotjs_gpio_t* gpio) {
  gpio->platform_data = IOTJS_ALLOC(iotjs_gpio_platform_data_t);
  gpio->platform_data->pin = INVALID_PIN;
#ifdef IOTJS_GPIO_ENABLE_IRQ
  gpio->platform_data->attach_irq = false;
#endif
}

void iotjs_gpio_destroy_platform_data(
    iotjs_gpio_platform_data_t* platform_data) {
#ifdef IOTJS_GPIO_ENABLE_IRQ
  if (platform_data->attach_irq)
    _gpio_irq_detach(platform_data);
#endif
  IOTJS_RELEASE(platform_data);
}

bool iotjs_gpio_write(iotjs_gpio_t* gpio) {
  DDDLOG("%s - pin: %d, value: %d", __func__, gpio->pin, gpio->value);
  os_pin_write(gpio->pin, gpio->value);

  return true;
}

bool iotjs_gpio_read(iotjs_gpio_t* gpio) {
  DDDLOG("%s - pin: %d", __func__, gpio->pin);

  gpio->value = os_pin_read(gpio->pin);
  return true;
}

bool iotjs_gpio_close(iotjs_gpio_t* gpio) {
  if (gpio->platform_data->pin != INVALID_PIN) {
    gpio->platform_data->pin = INVALID_PIN;
    _gpio_deinit(gpio);
    return true;
  } else {
    return false;
  }
}

bool iotjs_gpio_set_direction(iotjs_gpio_t* gpio) {
  uint32_t old_pin = (uint32_t)gpio->platform_data->pin;

  if (old_pin != gpio->pin)
    return false;

  return _gpio_init(gpio);
}

bool iotjs_gpio_open(iotjs_gpio_t* gpio) {
  DDDLOG("%s - pin: %d, direction: %d, mode: %d", __func__, gpio->pin,
         gpio->direction, gpio->mode);
  uint32_t old_pin = (uint32_t)gpio->platform_data;

  if (old_pin == gpio->pin)
    return false;

  return _gpio_init(gpio);
}
