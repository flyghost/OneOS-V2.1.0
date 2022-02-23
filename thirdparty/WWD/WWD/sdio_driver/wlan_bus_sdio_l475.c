#include <string.h>
#include <stdio.h>

#include <board.h>
#include <os_task.h>

#include <shell.h>
#include <wwd_dev.h>
#include "wwd_constants.h"

#define kNoErr                      0       //! No error occurred.
#define kGeneralErr                -1       //! General error.
#define kInProgressErr              1       //! Operation in progress.
#define kTimeoutErr                 -6722   //! Timeout occurred.

#define SECTOR_SIZE (512)

/*
 * SDIO specific constants
 */
typedef enum
{
    SDIO_CMD_0  =  0,
    SDIO_CMD_3  =  3,
    SDIO_CMD_5  =  5,
    SDIO_CMD_7  =  7,
    SDIO_CMD_52 = 52,
    SDIO_CMD_53 = 53,
    __MAX_VAL   = 64
} sdio_command_t;

typedef enum
{
    SDIO_BLOCK_MODE = ( 0 << 2 ), /* These are STM32 implementation specific */
    SDIO_BYTE_MODE  = ( 1 << 2 )  /* These are STM32 implementation specific */
} sdio_transfer_mode_t;

typedef enum
{
    SDIO_1B_BLOCK    =  1,
    SDIO_2B_BLOCK    =  2,
    SDIO_4B_BLOCK    =  4,
    SDIO_8B_BLOCK    =  8,
    SDIO_16B_BLOCK   =  16,
    SDIO_32B_BLOCK   =  32,
    SDIO_64B_BLOCK   =  64,
    SDIO_128B_BLOCK  =  128,
    SDIO_256B_BLOCK  =  256,
    SDIO_512B_BLOCK  =  512,
    SDIO_1024B_BLOCK = 1024,
    SDIO_2048B_BLOCK = 2048
} sdio_block_size_t;

typedef enum
{
    RESPONSE_NEEDED,
    NO_RESPONSE
} sdio_response_needed_t;


#if !(defined (MICO_DISABLE_MCU_POWERSAVE)) && !(defined (SDIO_1_BIT)) //SDIO 4 Bit mode and enable MCU powersave, need an OOB interrupt

int32_t host_enable_oob_interrupt( void )
{
    //platform_gpio_init( &wifi_sdio_pins[WIFI_PIN_SDIO_OOB_IRQ], INPUT_HIGH_IMPEDANCE );
    //platform_gpio_irq_enable( &wifi_sdio_pins[WIFI_PIN_SDIO_OOB_IRQ], IRQ_TRIGGER_RISING_EDGE, sdio_oob_irq_handler, 0 );
    return kNoErr;
}

uint8_t host_platform_get_oob_interrupt_pin( void )
{
    return 0;//MICO_WIFI_OOB_IRQ_GPIO_PIN;
}

#elif defined (MICO_DISABLE_MCU_POWERSAVE) && !(defined (SDIO_1_BIT)) //SDIO 4 Bit mode and disable MCU powersave, do not need OOB interrupt 

int32_t host_enable_oob_interrupt( void )
{
    return kNoErr;
}

uint8_t host_platform_get_oob_interrupt_pin( void )
{
    return 0;
}
#endif

#ifdef SDIO_1_BIT
static void sdio_int_pin_irq_handler( void* arg ) //SDIO 1 Bit mode
{
    UNUSED_PARAMETER(arg);
    platform_mcu_powersave_exit_notify( );
    wlan_notify_irq( );
}

bool host_platform_is_sdio_int_asserted(void)
{
    if ( platform_gpio_input_get( &wifi_sdio_pins[WIFI_PIN_SDIO_IRQ] ) == true) //SDIO INT pin is high
        return false;
    else
        return true; // SDIO D1 is low, data need read
}

int32_t host_enable_oob_interrupt( void )
{
    return kNoErr;
}

#endif

void host_platform_enable_high_speed_sdio( void )
{
	return;
}

int32_t host_platform_bus_init( void )
{
	return 0;
}

int32_t host_platform_bus_deinit( void )
{
	return 0;
}

static sdio_block_size_t find_optimal_block_size( uint32_t data_size )
{
    if ( data_size > (uint32_t) 256 )
        return SDIO_512B_BLOCK;
    if ( data_size > (uint32_t) 128 )
        return SDIO_256B_BLOCK;
    if ( data_size > (uint32_t) 64 )
        return SDIO_128B_BLOCK;
    if ( data_size > (uint32_t) 32 )
        return SDIO_64B_BLOCK;
    if ( data_size > (uint32_t) 16 )
        return SDIO_32B_BLOCK;
    if ( data_size > (uint32_t) 8 )
        return SDIO_16B_BLOCK;
    if ( data_size > (uint32_t) 4 )
        return SDIO_8B_BLOCK;
    if ( data_size > (uint32_t) 2 )
        return SDIO_4B_BLOCK;

    return SDIO_4B_BLOCK;
}

os_int32_t host_sdio_io_rw_direct(struct os_mmcsd_card *card, os_int32_t rw, os_int32_t arg, os_uint32_t cmd_code, os_uint8_t *pdata)
{
    struct os_mmcsd_cmd cmd;
    os_int32_t          err;

    OS_ASSERT(card != OS_NULL);

    memset(&cmd, 0, sizeof(struct os_mmcsd_cmd));

    cmd.cmd_code = SD_IO_RW_DIRECT;
    cmd.arg      = rw ? SDIO_ARG_CMD52_WRITE : SDIO_ARG_CMD52_READ;
    cmd.arg |= arg;
    cmd.flags = RESP_SPI_R5 | RESP_R5 | CMD_AC;

    err = mmcsd_send_cmd(card->host, &cmd, 0);
    if (err)
        return err;

    if (cmd.resp[0] & R5_ERROR)
        return OS_EIO;
    if (cmd.resp[0] & R5_FUNCTION_NUMBER)
        return OS_ERROR;
    if (cmd.resp[0] & R5_OUT_OF_RANGE)
        return OS_ERROR;

    if (!rw)
    {
        *pdata = cmd.resp[0] & 0xFF;
    }
    
    return 0;
}

os_int32_t host_sdio_io_rw_extended(
                               struct os_mmcsd_card *card,
                               os_int32_t            argument,
                               os_uint8_t *          buf,
                               uint16_t data_size)
{
    int block_mode = (argument >> 27) & 1;

    int block_size;
    int blocks;

    if (block_mode == 1)
    {
        blocks = argument & 0x1ff;
        if (blocks == 0)
            blocks = 512;

        block_size = 512;
    }
    else
    {
        blocks = 1;
        
        block_size = argument & 0x1ff;
        if (block_size == 0)
            block_size = 512;

        block_size = find_optimal_block_size(block_size);

        argument = argument & ~0x1ff;
        if (block_size < 512)
            argument |= block_size;
    }

    struct os_mmcsd_req  req;
    struct os_mmcsd_cmd  cmd;
    struct os_mmcsd_data data;

    OS_ASSERT(card != OS_NULL);

    memset(&req, 0, sizeof(struct os_mmcsd_req));
    memset(&cmd, 0, sizeof(struct os_mmcsd_cmd));
    memset(&data, 0, sizeof(struct os_mmcsd_data));

    req.cmd  = &cmd;
    req.cmd->data = &data;

    cmd.cmd_code = SD_IO_RW_EXTENDED;
    cmd.arg      = argument;
    cmd.flags = RESP_SPI_R5 | RESP_R5 | CMD_ADTC;

    data.blksize = block_size;
    data.blks    = blocks;
    data.flags   = (argument >> 31) ? DATA_DIR_WRITE : DATA_DIR_READ;
    data.buf     = buf;

    mmcsd_set_data_timeout(&data, card);

    mmcsd_send_request(card->host, &req);

    if (cmd.err)
        return cmd.err;
    if (data.err)
        return data.err;

    if (!controller_is_spi(card->host))
    {
        if (cmd.resp[0] & R5_ERROR)
            return OS_EIO;
        if (cmd.resp[0] & R5_FUNCTION_NUMBER)
            return OS_ERROR;
        if (cmd.resp[0] & R5_OUT_OF_RANGE)
            return OS_ERROR;
    }
    
    return 0;
}

extern struct os_wwd_device *wwd_dev;
OS_USED int32_t host_platform_sdio_transfer(wwd_bus_transfer_direction_t direction, sdio_command_t command, sdio_transfer_mode_t mode,
                                                sdio_block_size_t block_size, uint32_t argument, /*@null@*/ uint32_t* data, uint16_t data_size,
                                                    sdio_response_needed_t response_expected, /*@out@*/ /*@null@*/ uint32_t* response)
{   
    os_uint32_t  rw;
    int32_t result;

    if ( response != NULL )
    {
        *response = 0;
    }
    
    if (command == SDIO_CMD_53 )
    {
        result = host_sdio_io_rw_extended(wwd_dev->card, argument, (os_uint8_t *)data, data_size);
    }
    else
    {
        rw = direction ? SDIO_ARG_CMD52_WRITE : SDIO_ARG_CMD52_READ;
        result = host_sdio_io_rw_direct(wwd_dev->card, rw, argument, command, (os_uint8_t *)response);
    }
    
    return result;
}

