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
 * @file        rtc_setup.h
 *
 * @brief       This file provides rtc setup function declaration.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __RTC_SETUP_H__
#define __RTC_SETUP_H__

int rtc_setup_time(int argc, char *argv[]);
int rtc_setup_date(int argc, char *argv[]);
    
#endif
