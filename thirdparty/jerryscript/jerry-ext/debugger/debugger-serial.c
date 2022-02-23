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

#include "jerryscript-debugger-transport.h"
#include "jerryscript-ext/debugger.h"
#include "jext-common.h"

#if (defined (JERRY_DEBUGGER) && (JERRY_DEBUGGER == 1)) && !defined _WIN32

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
// #include <termios.h>
#include <stdlib.h>

#include <sys/ioctl.h>
#include "device.h"
#include "serial.h"


/* Max size of configuration string */
#define CONFIG_SIZE (32)

/**
 * Implementation of transport over serial connection.
 */
typedef struct
{
  jerry_debugger_transport_header_t header; /**< transport header */
  int fd; /**< file descriptor */
} jerryx_debugger_transport_serial_t;

/**
 * Correctly close a file descriptor.
 */
static inline void
jerryx_debugger_serial_close_fd (int fd) /**< file descriptor to close */
{
  if (close (fd) != 0)
  {
    JERRYX_ERROR_MSG ("Error while closing the file descriptor: %d\n", errno);
  }
} /* jerryx_debugger_serial_close_fd */

/**
 * Set a file descriptor to blocking or non-blocking mode.
 *
 * @return true if everything is ok
 *         false if there was an error
 **/
static bool
jerryx_debugger_serial_set_blocking (int fd, bool blocking)
{
  /* Save the current flags */
  int flags = fcntl (fd, F_GETFL, 0);
  if (flags == -1)
  {
    JERRYX_ERROR_MSG ("Error %d during get flags from file descriptor\n", errno);
    return false;
  }

  if (blocking)
  {
    flags &= ~O_NONBLOCK;
  }
  else
  {
    flags |= O_NONBLOCK;
  }

  if (fcntl (fd, F_SETFL, flags) == -1)
  {
    JERRYX_ERROR_MSG ("Error %d during set flags from file descriptor\n", errno);
    return false;
  }

  return true;
} /* jerryx_debugger_serial_set_blocking */

/**
 * Configure the file descriptor used by the serial communcation.
 *
 * @return true if everything is ok
 *         false if there was an error
 */
static inline bool
jerryx_debugger_serial_configure_attributes (int fd, struct serial_configure * serial_config)
{
  int err = ioctl(fd, OS_DEVICE_CTRL_CONFIG, serial_config);

  /* Flushes both data received but not read, and data written but not transmitted */
  if (err != 0)
  {
    JERRYX_ERROR_MSG ("Error %d in tcflush() :%s\n", errno, strerror (errno));
    jerryx_debugger_serial_close_fd (fd);
    return false;
  }

  return true;
} /* jerryx_debugger_serial_configure_attributes */

/**
 * Close a serial connection.
 */
static void
jerryx_debugger_serial_close (jerry_debugger_transport_header_t *header_p) /**< serial implementation */
{
  JERRYX_ASSERT (!jerry_debugger_transport_is_connected ());

  jerryx_debugger_transport_serial_t *serial_p = (jerryx_debugger_transport_serial_t *) header_p;

  JERRYX_DEBUG_MSG ("Serial connection closed.\n");

  jerryx_debugger_serial_close_fd (serial_p->fd);

  jerry_heap_free ((void *) header_p, sizeof (jerryx_debugger_transport_serial_t));
} /* jerryx_debugger_serial_close */

/**
 * Send data over a serial connection.
 *
 * @return true - if the data has been sent successfully
 *         false - otherwise
 */
static bool
jerryx_debugger_serial_send (jerry_debugger_transport_header_t *header_p, /**< serial implementation */
                             uint8_t *message_p, /**< message to be sent */
                             size_t message_length) /**< message length in bytes */
{
  JERRYX_ASSERT (jerry_debugger_transport_is_connected ());

  jerryx_debugger_transport_serial_t *serial_p = (jerryx_debugger_transport_serial_t *) header_p;

  do
  {
    ssize_t sent_bytes = write (serial_p->fd, message_p, message_length);

    if (sent_bytes < 0)
    {
      if (errno == EWOULDBLOCK)
      {
        continue;
      }

      JERRYX_ERROR_MSG ("Error: write to file descriptor: %d\n", errno);
      jerry_debugger_transport_close ();
      return false;
    }

    message_p += sent_bytes;
    message_length -= (size_t) sent_bytes;
  }
  while (message_length > 0);

  return true;
} /* jerryx_debugger_serial_send */

/**
 * Receive data from a serial connection.
 */
static bool
jerryx_debugger_serial_receive (jerry_debugger_transport_header_t *header_p, /**< serial implementation */
                                jerry_debugger_transport_receive_context_t *receive_context_p) /**< receive context */
{
  jerryx_debugger_transport_serial_t *serial_p = (jerryx_debugger_transport_serial_t *) header_p;

  uint8_t *buffer_p = receive_context_p->buffer_p + receive_context_p->received_length;
  size_t buffer_size = JERRY_DEBUGGER_TRANSPORT_MAX_BUFFER_SIZE - receive_context_p->received_length;

  ssize_t length = read (serial_p->fd, buffer_p, buffer_size);

  if (length <= 0)
  {
    if (errno != EWOULDBLOCK || length == 0)
    {
      jerry_debugger_transport_close ();
      return false;
    }
    length = 0;
  }

  receive_context_p->received_length += (size_t) length;

  if (receive_context_p->received_length > 0)
  {
    receive_context_p->message_p = receive_context_p->buffer_p;
    receive_context_p->message_length = receive_context_p->received_length;
  }

  return true;
} /* jerryx_debugger_serial_receive */

/**
 * Create a serial connection.
 *
 * @return true if successful,
 *         false otherwise
 */
bool
jerryx_debugger_serial_create (const char *config) /**< specify the configuration */
{
  /* Parse the configuration string */
  char tmp_config[CONFIG_SIZE];
  strncpy (tmp_config, config, CONFIG_SIZE);
  struct serial_configure configure;

  char *token = strtok (tmp_config, ",");
  char *device_id = token ? token : "uart3";
  configure.baud_rate = (token = strtok (NULL, ",")) ? (uint32_t) strtoul (token, NULL, 10) : 115200;
  configure.data_bits = (token = strtok (NULL, ",")) ? (uint32_t) strtoul (token, NULL, 10) : 8;
  configure.parity = PARITY_NONE;
  configure.stop_bits = (token = strtok (NULL, ",")) ? (uint32_t) strtoul (token, NULL, 10) : 1;
  configure.bit_order = BIT_ORDER_LSB;
  configure.invert = NRZ_NORMAL;
  configure.rx_bufsz = OS_SERIAL_RX_BUFSZ;
  configure.tx_bufsz = OS_SERIAL_TX_BUFSZ;
  configure.reserved = 0;

  int fd = open (device_id, O_RDWR);

  if (fd < 0)
  {
    JERRYX_ERROR_MSG ("Error %d opening %s: %s", errno, device_id, strerror (errno));
    return false;
  }

  if (!jerryx_debugger_serial_configure_attributes (fd, &configure))
  {
    jerryx_debugger_serial_close_fd (fd);
    return false;
  }

  JERRYX_DEBUG_MSG ("Waiting for client connection\n");

  /* Client will sent a 'c' char to initiate the connection. */
  uint8_t conn_char;
  ssize_t t = read (fd, &conn_char, 1);
  if (t != 1 || conn_char != 'c' || !jerryx_debugger_serial_set_blocking (fd, false))
  {
    return false;
  }

  JERRYX_DEBUG_MSG ("Client connected\n");

  size_t size = sizeof (jerryx_debugger_transport_serial_t);

  jerry_debugger_transport_header_t *header_p;
  header_p = (jerry_debugger_transport_header_t *) jerry_heap_alloc (size);

  if (!header_p)
  {
    jerryx_debugger_serial_close_fd (fd);
    return false;
  }

  header_p->close = jerryx_debugger_serial_close;
  header_p->send = jerryx_debugger_serial_send;
  header_p->receive = jerryx_debugger_serial_receive;

  ((jerryx_debugger_transport_serial_t *) header_p)->fd = fd;

  jerry_debugger_transport_add (header_p,
                                0,
                                JERRY_DEBUGGER_TRANSPORT_MAX_BUFFER_SIZE,
                                0,
                                JERRY_DEBUGGER_TRANSPORT_MAX_BUFFER_SIZE);

  return true;
} /* jerryx_debugger_serial_create */

#else /* !(defined (JERRY_DEBUGGER) && (JERRY_DEBUGGER == 1)) || _WIN32 */
/**
 * Dummy function when debugger is disabled.
 *
 * @return false
 */
bool
jerryx_debugger_serial_create (const char *config)
{
  JERRYX_UNUSED (config);
  return false;
} /* jerryx_debugger_serial_create */

#endif /* (defined (JERRY_DEBUGGER) && (JERRY_DEBUGGER == 1)) && !defined _WIN32 */
