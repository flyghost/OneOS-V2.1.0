/*
 * This file is part of the EasyFlash Library.
 *
 * Copyright (c) 2015-2019, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Portable interface for each platform.
 * Created on: 2015-01-16
 */
#include <board.h>
#include <easyflash.h>
#include <stdarg.h>
#include <sfud.h>
#include "spi_flash_sfud.h"


#define FAL_EF_PART_NAME               "easyflash"
static fal_part_t *part = NULL;

#ifdef NOR_FLASH
static const sfud_flash *    sfud_dev = NULL;
#endif

/* default environment variables set for user */
static const ef_env default_env_set[] = { 
    {"username", "test_dev", 0},                          // string env                   
    {"password", "12345678", 0},                          // string env
};

/**
 * Flash port for hardware initialize.
 *
 * @param default_env default ENV set for user
 * @param default_env_size default ENV size
 *
 * @return result
 */
EfErrCode ef_port_init(ef_env const **default_env, size_t *default_env_size) {
    EfErrCode result = EF_NO_ERR;

    *default_env = default_env_set;
    *default_env_size = sizeof(default_env_set) / sizeof(default_env_set[0]);

    #ifdef NOR_FLASH
    /* initialize SFUD library for SPI Flash */
    sfud_dev = os_sfud_flash_find(OS_EXTERN_FLASH_BUS_NAME);
    if (sfud_dev !=  NULL)
    {
        os_kprintf("init spi device successfully.\r\n");
    }
    #endif 

    part = fal_part_find(FAL_EF_PART_NAME);
    EF_ASSERT(part);
    
    return result;
}

/**
 * Read data from flash.
 * @note This operation's units is word.
 *
 * @param addr flash address
 * @param buf buffer to store read data
 * @param size read bytes size
 *
 * @return result
 */
EfErrCode ef_port_read(uint32_t addr, uint32_t *buf, size_t size) {
    EfErrCode result = EF_NO_ERR;

    /* You can add your code under here. */
    fal_part_read(part, addr, (uint8_t *)buf, size);
    
    return result;
}

/**
 * Erase data on flash.
 * @note This operation is irreversible.
 * @note This operation's units is different which on many chips.
 *
 * @param addr flash address
 * @param size erase bytes size
 *
 * @return result
 */
EfErrCode ef_port_erase(uint32_t addr, size_t size) {
    EfErrCode result = EF_NO_ERR;

    /* make sure the start address is a multiple of EF_ERASE_MIN_SIZE */
    EF_ASSERT(addr % EF_ERASE_MIN_SIZE == 0);
   
    /* You can add your code under here. */  
    if (fal_part_erase(part, addr, size) < 0)
    {
        result = EF_ERASE_ERR;
    }
    
    return result;
}

/**
 * Write data to flash.
 * @note This operation's units is word.
 * @note This operation must after erase. @see flash_erase.
 *
 * @param addr flash address
 * @param buf the write data buffer
 * @param size write bytes size
 *
 * @return result
 */
EfErrCode ef_port_write(uint32_t addr, const uint32_t *buf, size_t size) {
    EfErrCode result = EF_NO_ERR; 
   
    /* You can add your code under here. */ 
    #ifdef USING_LOGIC_FALSH
    /* Some flash can not write data when unit was written 0xFF already. */
    /* Using a logic flash(malloc from memory) to store data as a copy and erase flash before write. */ 
    
    /* malloc memory sapce for logic flash */
    uint8_t *backup_data = NULL;
    backup_data = malloc(EF_ERASE_MIN_SIZE);   
    if (backup_data == NULL)
    {
        os_kprintf("logic flash malloc failed.\r\n");
    }
 
    /* copy flash-data to memory(logic flash) as copy-data */
    uint32_t erase_addr;
    erase_addr = addr - (addr & (EF_ERASE_MIN_SIZE - 1));
    fal_part_read(part, erase_addr, (uint8_t *)backup_data, EF_ERASE_MIN_SIZE);
    
    /* merge data(need to write) to copy-data */
    uint32_t merge_offset = addr - erase_addr;
    memcpy(&backup_data[merge_offset],(uint32_t *)buf,size);

    /* erase sector of flash */
    if (fal_part_erase(part, erase_addr, EF_ERASE_MIN_SIZE) < 0)
    {
        result = EF_ERASE_ERR;
    }
    
    /* write all data to flash */
    if (fal_part_write(part, erase_addr, (uint8_t *)backup_data, EF_ERASE_MIN_SIZE) < 0)
    {
        result = EF_WRITE_ERR;
    }
    
    /* free memory sapce */
    free(backup_data);   
    
    #else
    if (fal_part_write(part, addr, (uint8_t *)buf, size) < 0)
    {
        result = EF_WRITE_ERR;
    }
    #endif
 
    return result;
}

/**
 * lock the ENV ram cache
 */
void ef_port_env_lock(void) {
    
    /* You can add your code under here. */
    
}

/**
 * unlock the ENV ram cache
 */
void ef_port_env_unlock(void) {
    
    /* You can add your code under here. */
    
}


static char log_buf[128];

/**
 * This function is print flash debug info.
 *
 * @param file the file which has call this function
 * @param line the line number which has call this function
 * @param format output format
 * @param ... args
 *
 */
void ef_log_debug(const char *file, const long line, const char *format, ...) {

#ifdef PRINT_DEBUG

    va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);

    /* You can add your code under here. */
    
    va_end(args);

#endif

}

/**
 * This function is print flash routine info.
 *
 * @param format output format
 * @param ... args
 */
void ef_log_info(const char *format, ...) {
    va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);

    /* You can add your code under here. */
    os_vsnprintf(log_buf, (os_size_t) - 1, format, args);
    os_kprintf("%s", log_buf);
    
    va_end(args);
}

/**
 * This function is print flash non-package info.
 *
 * @param format output format
 * @param ... args
 */
void ef_print(const char *format, ...) {
    va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);
    
    /* You can add your code under here. */
    os_vsnprintf(log_buf, (os_size_t) - 1, format, args);
    os_kprintf("%s", log_buf);
    
    va_end(args);   
}
