/* Copyright 2015-present Samsung Electronics Co., Ltd. and other contributors
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


#include "iotjs_def.h"
#include "iotjs_util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__linux__) && !defined(__OPENWRT__)
#include <execinfo.h>
#endif

void force_terminate(void);

iotjs_string_t iotjs_file_read(const char* path) {
  FILE* file = fopen(path, "rb");
  if (file == NULL) {
    iotjs_string_t empty_content = iotjs_string_create();
    return empty_content;
  }

  int fseek_ret = fseek(file, 0, SEEK_END);
  IOTJS_ASSERT(fseek_ret == 0);
  long ftell_ret = ftell(file);
  IOTJS_ASSERT(ftell_ret >= 0);
  size_t len = (size_t)ftell_ret;
  fseek_ret = fseek(file, 0, SEEK_SET);
  IOTJS_ASSERT(fseek_ret == 0);

  if (ftell_ret < 0 || fseek_ret != 0) {
    iotjs_string_t empty_content = iotjs_string_create();
    fclose(file);
    DLOG("iotjs_file_read error");
    return empty_content;
  }

  char* buffer = iotjs_buffer_allocate(len + 1);

#if defined(__NUTTX__) || defined(__TIZENRT__)
  char* ptr = buffer;
  unsigned nread = 0;
  unsigned read = 0;

  while ((nread = fread(ptr, 1, IOTJS_MAX_READ_BUFFER_SIZE, file)) > 0) {
    read += nread;
    ptr = buffer + read;
  }
#else
  size_t read = fread(buffer, 1, len, file);
#endif
  IOTJS_ASSERT(read == len);

  *(buffer + len) = 0;

  fclose(file);

  iotjs_string_t contents = iotjs_string_create_with_buffer(buffer, len);

  return contents;
}

void iotjs_print_cur_bt() {
  if (!jerry_is_feature_enabled (JERRY_FEATURE_LINE_INFO)) {
    jerry_port_log (JERRY_LOG_LEVEL_TRACE,
        "Line info disabled, no backtrace will be printed\r\n");
    return;
  }

  jerry_value_t backtrace_array = jerry_get_backtrace (10);
  uint32_t array_length = jerry_get_array_length (backtrace_array);

  for (uint32_t idx = 0; idx < array_length; idx++) {
    jerry_value_t property = jerry_get_property_by_index (backtrace_array, idx);

    jerry_char_t string_buffer[64];
    jerry_size_t copied_bytes = jerry_substring_to_char_buffer (property,
                                                                0,
                                                                63,
                                                                string_buffer,
                                                                63);
    string_buffer[copied_bytes] = '\0';
    jerry_port_log (JERRY_LOG_LEVEL_TRACE,
        " %d: %s\r\n", idx, string_buffer);

    jerry_release_value (property);
  }

  jerry_release_value (backtrace_array);
}

void iotjs_print_value(jerry_value_t v) {
  jerry_value_t str_val;

  if (jerry_value_is_symbol (v))
    str_val = jerry_get_symbol_descriptive_string (v);
  else
    str_val = jerry_value_to_string (v);

  if (! jerry_value_is_error (str_val)) {
    jerry_length_t length = jerry_get_utf8_string_length (str_val);
    jerry_char_t substr_buf[256];

    jerry_size_t substr_size = jerry_substring_to_utf8_char_buffer (str_val,
                                                                    0,
                                                                    length,
                                                                    substr_buf,
                                                                    256 - 1);

    jerry_char_t *buf_end_p = substr_buf + substr_size;
    *buf_end_p++ = '\n';

    for (jerry_char_t *buf_p = substr_buf; buf_p < buf_end_p; buf_p++) {
      char chr = (char) *buf_p;

      if (chr != '\0') {
        jerry_port_print_char (chr);
      }
    }
  }

  jerry_release_value (str_val);
} /* jerryx_handler_print */

char* iotjs_buffer_allocate(size_t size) {
  char* buffer = (char*)(jerry_port_memalloc(size * sizeof(char)));
  if (buffer == NULL) {
    DLOG("Out of memory");
    force_terminate();
  }
  return buffer;
}


char* iotjs_buffer_allocate_from_number_array(size_t size,
                                              const jerry_value_t array) {
  char* buffer = iotjs_buffer_allocate(size);
  for (size_t i = 0; i < size; i++) {
    jerry_value_t jdata = iotjs_jval_get_property_by_index(array, i);
    buffer[i] = iotjs_jval_as_number(jdata);
    jerry_release_value(jdata);
  }
  return buffer;
}


char* iotjs_buffer_reallocate(char* buffer, size_t size) {
  IOTJS_ASSERT(buffer != NULL);
  char* newbuffer = (char*)(jerry_port_realloc(buffer, size));
  if (newbuffer == NULL) {
    DLOG("Out of memmory");
    force_terminate();
  }
  return newbuffer;
}


void iotjs_buffer_release(char* buffer) {
  if (buffer) {
    jerry_port_memfree(buffer);
  }
}

void print_stacktrace(void) {
#if !defined(NDEBUG) && defined(__linux__) && defined(DEBUG) && \
    !defined(__OPENWRT__)
  // TODO: support other platforms
  const int numOfStackTrace = 100;
  void* buffer[numOfStackTrace];
  char command[256];

  int nptrs = backtrace(buffer, numOfStackTrace);
  char** strings = backtrace_symbols(buffer, nptrs);

  if (strings == NULL) {
    perror("backtrace_symbols");
    exit(EXIT_FAILURE);
  }

  printf("\n[Backtrace]:\n");
  for (int j = 0; j < nptrs - 2; j++) { // remove the last two
    int idx = 0;
    while (strings[j][idx] != '\0') {
      if (strings[j][idx] == '(') {
        break;
      }
      idx++;
    }
    snprintf(command, sizeof(command), "addr2line %p -e %.*s", buffer[j], idx,
             strings[j]);

    if (system(command)) {
      break;
    }
  }

  free(strings);
#endif // !defined(NDEBUG) && defined(__linux__) && defined(DEBUG) &&
       // !defined(__OPENWRT__)
}

void force_terminate(void) {
  exit(EXIT_FAILURE);
}
