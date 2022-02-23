/**
 ***********************************************************************************************************************
 * Copyright (c) 2021, China Mobile Communications Group Co.,Ltd.
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
 * @file        ope_operating_sys.h
 * 
 * @brief       Platform definition 
 * 
 * @details     Define the platform on which the current system resides
 * 
 * @revision
 * Date         Author          Notes
 * 2021-04-29   HuSong          First Version
 ***********************************************************************************************************************
 */

#ifndef __OPE_OPERATING_SYS_H__
#define __OPE_OPERATING_SYS_H__

#if defined WIN32 || _WIN32 || _WIN64 || _WINDOWS || _MSC_VER
#define _OPE_WINDOWS_ /* Windows platform */
#elif defined linux || __linux || __linux__
#define _OPE_LINUX_   /* Linux platform */
#else
#define _OPE_ONEOS_   /* OneOS platform */
#endif

#endif /* __OPE_OPERATING_SYS_H__ */