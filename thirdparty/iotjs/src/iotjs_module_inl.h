/* File generated */
extern jerry_value_t iotjs_init_fs(void);
extern jerry_value_t iotjs_init_uart(void);
extern jerry_value_t iotjs_init_dns(void);
extern jerry_value_t iotjs_init_buffer(void);
extern jerry_value_t iotjs_init_process(void);
extern jerry_value_t iotjs_init_i2c(void);
extern jerry_value_t iotjs_init_udp(void);
extern jerry_value_t iotjs_init_console(void);
extern jerry_value_t iotjs_init_http_parser(void);
extern jerry_value_t iotjs_init_timer(void);
extern jerry_value_t iotjs_init_gpio(void);
extern jerry_value_t iotjs_init_pwm(void);
extern jerry_value_t iotjs_init_constants(void);
extern jerry_value_t iotjs_init_tcp(void);

const unsigned iotjs_module_count = 14;

const
iotjs_module_ro_data_t iotjs_module_ro_data[14] = {
  { "fs", iotjs_init_fs },
  { "uart", iotjs_init_uart },
  { "dns", iotjs_init_dns },
  { "buffer", iotjs_init_buffer },
  { "process", iotjs_init_process },
  { "i2c", iotjs_init_i2c },
  { "udp", iotjs_init_udp },
  { "console", iotjs_init_console },
  { "http_parser", iotjs_init_http_parser },
  { "timers", iotjs_init_timer },
  { "gpio", iotjs_init_gpio },
  { "pwm", iotjs_init_pwm },
  { "constants", iotjs_init_constants },
  { "tcp", iotjs_init_tcp },
};

iotjs_module_rw_data_t iotjs_module_rw_data[14] = {
  { 0 },
  { 0 },
  { 0 },
  { 0 },
  { 0 },
  { 0 },
  { 0 },
  { 0 },
  { 0 },
  { 0 },
  { 0 },
  { 0 },
  { 0 },
  { 0 },

};
