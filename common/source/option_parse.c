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
 * @file        option_parse.c
 *
 * @brief       This file implements command option parser.
 *
 * @details     This command option parser doesn't support "--" option.            
 *
 * @revision
 * Date         Author          Notes
 * 2020-07-02   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#include <os_types.h>
#include <os_stddef.h>
#include <os_assert.h>
#include <option_parse.h>
#include <os_util.h>
#include <string.h>

/**
 ***********************************************************************************************************************
 * @brief           Initialize command option parser.      
 *
 * @attention       .
 *
 * @param[in]       state           Option parser state.
 * @param[in]       begin_index     The first option index in "argv" argument vector.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void opt_init(opt_state_t *state, os_int32_t begin_index)
{
    OS_ASSERT(state);

    state->opt_index = begin_index;
    state->opt_char  = 0;
    state->opt_arg   = OS_NULL;
    state->opt_place = OS_NULL;

    return;
}

/**
 ***********************************************************************************************************************
 * @brief           This function implements command option parser.
 *
 * @details         The function shall return the next option character (if one is found) from argv that matches a 
 *                  character in optstring. If the option takes an argument, opt_get() shall set the variable
 *                  state->opt_arg to point to the option-argument. 
 *
 * @attention       opt_init() must be called before using this function.
 *
 * @param[in]       argc            Argument count.
 * @param[in]       argv            Argument.
 * @param[in]       optstring       Is a string of recognized option characters.
 * @param[in]       state           Option parser state.
 *
 * @return          Next option character (if one is found) from argv that matches a character in optstring. 
 * @retval          OPT_EOF         End options parsing.
 * @retval          OPT_BADOPT      Bad option.
 * @retval          OPT_BADARG      Bad argument.
 * @retval          else            The next option character.
 ***********************************************************************************************************************
 */
os_int32_t opt_get(os_int32_t argc, char * const *argv, const char *optstring, opt_state_t *state)
{
    char *oli;      /* Option letter list index */

    if (state->opt_index >= argc)
    {
        return OPT_EOF;
    }

    state->opt_place = argv[state->opt_index];
    if (*state->opt_place != '-')
    {
        os_kprintf("Illegal option: %s\r\n", state->opt_place);
        return OPT_BADOPT;
    }

    if (state->opt_place[1])
    { 
        if (state->opt_place[1] == '-')
        {
            /* Found "--" */
            os_kprintf("Not support \"--\" option(%s)\r\n", argv[state->opt_index]);
            return OPT_BADOPT;
        }

        state->opt_place++;
    }
    else
    {
        os_kprintf("Illegal option: %s\r\n", argv[state->opt_index]);
        return OPT_BADOPT;
    }

    /* Process option letter */
    state->opt_char = (os_int32_t)(*state->opt_place);
    oli = strchr(optstring, (char)state->opt_char);

    if ((state->opt_char == (os_int32_t)':') || !oli)
    {
        os_kprintf("Illegal option: %s\r\n", argv[state->opt_index]);
        return OPT_BADOPT;
    }

    state->opt_place++;
    if (*state->opt_place)
    {
        os_kprintf("Illegal option: %s\r\n", argv[state->opt_index]);
        return OPT_BADOPT;   
    }
    
    oli++;

    /* Don't need argument */
    if (*oli != ':')
    {
        state->opt_arg = OS_NULL;   
    }
    /* Need an argument */
    else
    {
        /* No arg */
        if (argc <= state->opt_index + 1)
        {
            os_kprintf("Option(%s) requires an argument\r\n", argv[state->opt_index]);
            return OPT_BADARG;
        }

        state->opt_index++;
        state->opt_arg = argv[state->opt_index];    
    }

    state->opt_index++;

    return state->opt_char;
}

