/* Copyright JS Foundation and other contributors, http://js.foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdarg.h>
#include <sys/time.h>

#include "oneos_config.h"
#include "os_util.h"
#include "dlog.h"
#include "os_memory.h"

#include "jerryscript-port.h"

void jerry_port_log (jerry_log_level_t level,
                const char *format,
                ...)
{
  static char log_buf[OS_LOG_BUFF_SIZE];

  va_list args;
  va_start(args, format);
  os_vsnprintf(log_buf, OS_LOG_BUFF_SIZE, format, args);
  va_end(args);

  dlog_raw("%s", log_buf);
} /* jerry_port_log */

void jerry_port_fatal (jerry_fatal_code_t code)
{
  jerry_port_log (JERRY_LOG_LEVEL_ERROR, "Jerry Fatal Error! err code : %d\r\n", code);
  while (true);
} /* jerry_port_fatal */

double jerry_port_get_current_time (void)
{
  struct timeval tv;

  if (gettimeofday (&tv, NULL) == 0)
  {
    return ((double) tv.tv_sec) * 1000.0 + ((double) tv.tv_usec) / 1000.0;
  }
} /* jerry_port_get_current_time */

double jerry_port_get_local_time_zone_adjustment (double unix_ms, bool is_utc)
{
  /* We live in UTC. */
  return 0;
} /* jerry_port_get_local_time_zone_adjustment */

void jerry_port_print_char (char c)
{
  os_kprintf ("%c", c);
} /* jerry_port_print_char */

void * jerry_port_memalloc(size_t size)
{
    return os_aligned_malloc(8, size);
}

void * jerry_port_realloc(void *ptr, size_t size)
{
    return os_realloc(ptr, size);
}

void jerry_port_memfree(void *ptr)
{
    os_free(ptr);
}

#if ENABLED (JERRY_DEBUGGER)
void jerry_port_sleep(uint32_t sleep_time) {
  os_task_msleep(sleep_time);
} /* jerry_port_sleep */
#endif /* JERRY_DEBUGGER */
