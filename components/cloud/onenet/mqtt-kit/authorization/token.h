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
 * @file        token.h
 * 
 * @brief       Supply functions to calculate OneNET-MQTTS token, which use in device register and conncet.  
 * 
 * @details     
 * 
 * @revision
 * Date         Author          Notes
 * 2020-06-08   OneOs Team      First Version
 ***********************************************************************************************************************
 */

#ifndef _TOKEN_H_
#define _TOKEN_H_

extern int onenet_authorization(char          *ver,
                                char          *res,
                                unsigned int   et,
                                char          *access_key,
                                char          *dev_name,
                                char          *authorization_buf,
                                unsigned short authorization_buf_len,
                                _Bool          flag);

#endif /* _TOKEN_H_ */
