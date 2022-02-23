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
 * @file        option_parse.h
 *
 * @brief       Header file for command option parser.
 *
 * @revision
 * Date         Author          Notes
 * 2020-07-02   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef __OPTION_PARSE_H__
#define __OPTION_PARSE_H__

#include <os_types.h>

#define OPT_EOF              (-1)
#define	OPT_BADOPT	        (int)'?'
#define	OPT_BADARG	        (int)':'          

struct opt_state
{
    os_int32_t  opt_index;          /* Index into parent argv vector */
    os_int32_t  opt_char;           /* Character checked for validity */
    char       *opt_arg;            /* Argument associated with option */
    char       *opt_place;          /* Additional persistent state */
};
typedef struct opt_state opt_state_t;

extern void        opt_init(opt_state_t *state, os_int32_t begin_index);
extern os_int32_t  opt_get(os_int32_t argc, char * const argv[], const char *optstring, opt_state_t *state);


#endif /* __OPTION_PARSE_H__ */

