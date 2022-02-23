/*
 * Copyright (c) 2021 OneOS Team
 */

#include <string.h>
#include <os_sem.h>
#include <os_clock.h>

#include "oc_api.h"
#include "port/oc_clock.h"
//#include <signal.h>
//#include <windows.h>

//int quit = 0;

static os_sem_t block;


static bool state = false;
int power;
oc_string_t name;

static int
app_init(void)
{
  int ret = oc_init_platform("Intel", NULL, NULL);
  ret |= oc_add_device("/oic/d", "oic.d.light", "Sword's Lamp", "ocf.1.0.0",
                       "ocf.res.1.0.0", NULL, NULL);
  oc_new_string(&name, "Sword's Light", 12);
  return ret;
}

static void
get_light(oc_request_t *request, oc_interface_mask_t iface_mask,
          void *user_data)
{
  (void)user_data;
  ++power;

  PRINT("GET_light:\n");
  oc_rep_start_root_object();
  switch (iface_mask) {
  case OC_IF_BASELINE:
    oc_process_baseline_interface(request->resource);
  case OC_IF_RW:
    oc_rep_set_boolean(root, state, state);
    oc_rep_set_int(root, power, power);
    oc_rep_set_text_string(root, name, oc_string(name));
    break;
  default:
    break;
  }
  oc_rep_end_root_object();
  oc_send_response(request, OC_STATUS_OK);
}


#include <board.h>
static void set_led(void)
{
  int i = 0;
  for (i = 0; i < led_table_size; i++)
  {
    if (state)
      os_pin_write(led_table[i].pin, led_table[i].active_level);
    else
      os_pin_write(led_table[i].pin, !led_table[i].active_level);
  }
}

static void
post_light(oc_request_t *request, oc_interface_mask_t iface_mask,
           void *user_data)
{
  (void)iface_mask;
  (void)user_data;
  PRINT("POST_light:\n");
  oc_rep_t *rep = request->request_payload;
  while (rep != NULL) {
    PRINT("key: %s ", oc_string(rep->name));
    switch (rep->type) {
    case OC_REP_BOOL:
      state = rep->value.boolean;
      PRINT("value: %d\n", state);
      set_led();
      break;
    case OC_REP_INT:
      power = (int)rep->value.integer;
      PRINT("value: %d\n", power);
      break;
    case OC_REP_STRING:
      oc_free_string(&name);
      oc_new_string(&name, oc_string(rep->value.string),
                    oc_string_len(rep->value.string));
      break;
    default:
      oc_send_response(request, OC_STATUS_BAD_REQUEST);
      return;
    }
    rep = rep->next;
  }
  oc_send_response(request, OC_STATUS_CHANGED);
}

static void
put_light(oc_request_t *request, oc_interface_mask_t iface_mask,
          void *user_data)
{
  (void)iface_mask;
  (void)user_data;
  post_light(request, iface_mask, user_data);
}

static void
register_resources(void)
{
  oc_resource_t *res = oc_new_resource(NULL, "/a/light", 2, 0);
  oc_resource_bind_resource_type(res, "core.light");
  oc_resource_bind_resource_type(res, "core.brightlight");
  oc_resource_bind_resource_interface(res, OC_IF_RW);
  oc_resource_set_default_interface(res, OC_IF_RW);
  oc_resource_set_discoverable(res, true);
  oc_resource_set_periodic_observable(res, 1);
  oc_resource_set_request_handler(res, OC_GET, get_light, NULL);
  oc_resource_set_request_handler(res, OC_PUT, put_light, NULL);
  oc_resource_set_request_handler(res, OC_POST, post_light, NULL);
  oc_add_resource(res);
}

static void
signal_event_loop(void)
{
  os_sem_post(&block);
}

//void
//handle_signal(int signal)
//{
//  signal_event_loop();
//  quit = 1;
//}

//int
//main(void)
  oc_clock_time_t next_tick = 0;
  oc_clock_time_t now_tick;
void ocf_server_init(void)
{
//  InitializeCriticalSection(&cs);
//  InitializeConditionVariable(&cv);

  int init;

  /* set the latency to 240 seconds*/
  /* if no latency is needed then remove the next line */
  //oc_core_set_latency(240);
  oc_set_max_app_data_size(0x4000);
	
//  signal(SIGINT, handle_signal);

  static const oc_handler_t handler = { .init = app_init,
                                        .signal_event_loop = signal_event_loop,
                                        .register_resources = register_resources };
  os_sem_init(&block, "ocf_sem", 0, 1);

#ifdef OC_STORAGE
  oc_storage_config("");
#endif /* OC_STORAGE */

  if (oc_main_init(&handler) < 0)
    return;


  int wait_tick;

  while (true) {
    next_tick = oc_main_poll();
    if (next_tick == 0)
      wait_tick = OS_WAIT_FOREVER;
    else
    {
      now_tick = oc_clock_time();
      wait_tick = next_tick - now_tick;
      if (wait_tick < 0)
        wait_tick = OS_WAIT_FOREVER;
    } 
    os_sem_wait(&block, wait_tick);
  }

  oc_main_shutdown();
}
