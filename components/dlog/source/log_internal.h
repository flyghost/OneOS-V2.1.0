/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with 
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on 
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the 
 * specific language governing permissions and limitations under the License.
 *
 * @file        log_internal.h
 *
 * @brief       Header file for dlog internal use.
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-24   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef __LOG_INTERNAL_H__
#define __LOG_INTERNAL_H__

extern void dlog_voutput(os_uint16_t level, const char *tag, os_bool_t newline, const char *format, va_list args);

#endif /* __LOG_INTERNAL_H__ */

