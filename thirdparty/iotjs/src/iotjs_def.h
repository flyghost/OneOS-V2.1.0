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

#ifndef IOTJS_DEF_H
#define IOTJS_DEF_H

#include "jerryscript-config.h"
#include "iotjs_module_define.h"

#define ENABLE_SNAPSHOT
#define EXPOSE_GC (1)

#ifndef IOTJS_MAX_READ_BUFFER_SIZE
#define IOTJS_MAX_READ_BUFFER_SIZE 1023
#define IOTJS_MAX_PATH_SIZE 120
#endif /* IOTJS_MAX_READ_BUFFER_SIZE */

#ifndef IOTJS_ASSERT
#ifdef NDEBUG
#define IOTJS_ASSERT(x) ((void)(x))
#else /* !NDEBUG */
extern void print_stacktrace(void);
extern void force_terminate(void);
#define IOTJS_ASSERT(x)                                                      \
  do {                                                                       \
    if (!(x)) {                                                              \
      fprintf(stderr, "%s:%d: Assertion '%s' failed.\n", __FILE__, __LINE__, \
              #x);                                                           \
      print_stacktrace();                                                    \
      force_terminate();                                                     \
    }                                                                        \
  } while (0)
#endif /* NDEBUG */
#endif /* IOTJS_ASSERT */

#if defined(__arm__)
#define TARGET_ARCH "arm"
#elif defined(__i686__)
#define TARGET_ARCH "ia32"
#elif defined(__x86_64__)
#define TARGET_ARCH "x64"
#else /* !__arm__ && !__i686__ && !__x86_64__ */
#define TARGET_ARCH "unknown"
#endif /* __arm__ */

#define TARGET_OS "oneos"

#define IOTJS_VERSION "1.0.0"

#if !defined(STRINGIFY)
#define STRINGIFY(x) #x
#endif /* STRINGIFY */

#if !defined(TOSTRING)
#define TOSTRING(x) STRINGIFY(x)
#endif /* TOSTRING */

#if !defined(TARGET_BOARD)
#define TARGET_BOARD "unknown"
#endif /* TARGET_BOARD */

#define NODE_MAJOR_VERSION 1
#define NODE_MINOR_VERSION 0
#define NODE_PATCH_VERSION 0

/* Avoid compiler warnings if needed. */
#define IOTJS_UNUSED(x) ((void)(x))

#define IOTJS_DEFINE_NATIVE_HANDLE_INFO_THIS_MODULE(name)                  \
  static void iotjs_##name##_destroy(iotjs_##name##_t* wrap);              \
  static const jerry_object_native_info_t this_module_native_info = {      \
    .free_cb = (jerry_object_native_free_callback_t)iotjs_##name##_destroy \
  }

#include <uv.h>
#include <assert.h>
#include <limits.h> /* PATH_MAX */
#include <stdbool.h>
#include <string.h>

// commonly used header files
#include "iotjs_binding.h"
#include "iotjs_binding_helper.h"
#include "iotjs_debuglog.h"
#include "iotjs_env.h"
#include "iotjs_magic_strings.h"
#include "iotjs_module.h"
#include "iotjs_string.h"
#include "iotjs_util.h"


#endif /* IOTJS_DEF_H */
