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

#include <string.h>
#include <board.h>
#include <driver.h>
#include <device.h>
#include <os_task.h>

char *oneos_getline(uint32_t timeout)
{
  static char recv_buf[32];
  int buf_size = sizeof(recv_buf) - 1;

  os_device_t *console = os_console_get_device();
  unsigned char *ptr = &recv_buf[0];

  do
  {
    if(os_device_read_nonblock(console, 0, ptr, 1) == 0)
    {
      os_task_msleep(1);
      timeout--;
      if(0 == timeout)
      {
        break;
      }
    }
    else
    {
      os_device_write_nonblock(console, 0, ptr, 1);
      if (*ptr == '\n') 
      {
        *(ptr + 1) = '\0';
        return recv_buf;
      }
      ptr++;
      buf_size--;
    }
  } while(buf_size);

  return NULL;
}

void oneos_getline_init(void)
{

}
