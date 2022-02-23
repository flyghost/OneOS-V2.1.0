/*
 * Copyright 2018, Cypress Semiconductor Corporation or a subsidiary of 
 * Cypress Semiconductor Corporation. All Rights Reserved.
 * 
 * This software, associated documentation and materials ("Software"),
 * is owned by Cypress Semiconductor Corporation
 * or one of its subsidiaries ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products. Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 */
#include "network/wwd_network_constants.h"
#include "network/wwd_buffer_interface.h"
#include "platform/wwd_bus_interface.h"
#include "lwip/netbuf.h"
#include "lwip/memp.h"
#include <string.h> /* for NULL */
#include "wwd_assert.h"
#include "RTOS/wwd_rtos_interface.h"
#include "wiced_utilities.h"
#include "wwd_wifi.h"

void memp_free_notify( unsigned int type );

wwd_result_t wwd_buffer_init( /*@null@*/ /*@unused@*/ void * native_arg )
{
    UNUSED_PARAMETER( native_arg );

    return WWD_SUCCESS;
}

wwd_result_t host_buffer_check_leaked( void )
{

    return WWD_SUCCESS;
}

wwd_result_t host_buffer_add_application_defined_pool( void* pool_in, wwd_buffer_dir_t direction )
{

    return WWD_SUCCESS;
}

wwd_result_t internal_host_buffer_get( wiced_buffer_t * buffer, wwd_buffer_dir_t direction, unsigned short size, unsigned long timeout_ms )
{
    UNUSED_PARAMETER( direction );

    wiced_assert("Error: Invalid buffer size\n", size != 0);

    *buffer = NULL;

    if ( size > (unsigned short) WICED_LINK_MTU )
    {
        WPRINT_NETWORK_DEBUG(("Attempt to allocate a buffer larger than the MTU of the link\n"));
        /*@-compdef@*/ /* Lint: buffer is not allocated in error case */
        return WWD_BUFFER_UNAVAILABLE_PERMANENT;
        /*@+compdef@*/
    }

    do
    {
        *buffer = pbuf_alloc( PBUF_RAW, size, PBUF_RAM);
        if ( *buffer != NULL )
        {
            break;
        }

        host_rtos_delay_milliseconds( 1 );

    } while ( timeout_ms-- > 0);

    if ( *buffer == NULL )
    {
        return WWD_BUFFER_UNAVAILABLE_TEMPORARY;
    }

    /*@-compdef@*/ /* Lint does not realise allocation has occurred */
    return WWD_SUCCESS;
    /*@+compdef@*/
}

wwd_result_t host_buffer_get( /*@special@*/ /*@out@*/ wiced_buffer_t* buffer, wwd_buffer_dir_t direction, unsigned short size, wiced_bool_t wait ) /*@allocates *buffer@*/  /*@defines **buffer@*/
{
    unsigned long wait_option = ( wait == WICED_TRUE ) ? (unsigned long) WICED_NEVER_TIMEOUT : 0;
    return internal_host_buffer_get(buffer, direction, size, wait_option);
}

#include <os_memory.h>
wwd_result_t host_protocol_buffer_get(wiced_buffer_t *buffer)
{
    *buffer = (wiced_buffer_t)os_calloc(1, sizeof(struct pbuf));
    if (*buffer == OS_NULL)
        return WWD_BUFFER_UNAVAILABLE_TEMPORARY;
    
    return WWD_SUCCESS;
}


void host_buffer_release( /*@only@*/ wiced_buffer_t buffer, wwd_buffer_dir_t direction )
{
    UNUSED_PARAMETER( direction );

    wiced_assert("Error: Invalid buffer\n", buffer != NULL);

    (void) pbuf_free( buffer );
}

/*@exposed@*/ uint8_t* host_buffer_get_current_piece_data_pointer( /*@temp@*/ wiced_buffer_t buffer )
{
    wiced_assert("Error: Invalid buffer\n", buffer != NULL);
    return (uint8_t*) buffer->payload;
}

uint16_t host_buffer_get_current_piece_size( /*@temp@*/ wiced_buffer_t buffer )
{
    wiced_assert("Error: Invalid buffer\n", buffer != NULL);
    return (uint16_t) buffer->tot_len;
}

/*@exposed@*/ /*@dependent@*/ /*@null@*/ wiced_buffer_t host_buffer_get_next_piece( /*@dependent@*/ wiced_buffer_t buffer )
{
    wiced_assert("Error: Invalid buffer\n", buffer != NULL);
    return buffer->next;
}

wwd_result_t host_buffer_add_remove_at_front( wiced_buffer_t * buffer, int32_t add_remove_amount )
{
    wiced_assert("Error: Invalid buffer\n", buffer != NULL);
    if ( (u8_t) 0 != pbuf_header( *buffer, (s16_t) ( -add_remove_amount ) ) )
    {
        WPRINT_NETWORK_DEBUG(("Failed to move pointer - usually because not enough space at front of buffer\n"));
        return WWD_BUFFER_POINTER_MOVE_ERROR;
    }

    return WWD_SUCCESS;
}

wiced_buffer_t host_buffer_pop_from_fifo( wiced_buffer_fifo_t* fifos, wwd_interface_t* interface_return )
{
    wiced_buffer_t buffer = NULL;
    int interface_index               = 0;
    wwd_interface_t current_interface = WWD_INDEX_TO_INTERFACE( interface_index );
    for (  ; interface_index < WWD_INTERFACE_MAX ; interface_index++, current_interface = WWD_INDEX_TO_INTERFACE( interface_index ) )
    {
        wiced_buffer_interface_fifo_t *fifo = &( fifos->per_interface_fifos[ WWD_INTERFACE_INDEX( current_interface ) ] );

        if ( fifo->first != NULL )
        {
            buffer         = fifo->first;
            if ( interface_return != NULL )
            {
                *interface_return = current_interface;
            }
            fifo->first    = buffer->next;
        }
    }
    return buffer;
}

void host_buffer_push_to_fifo( wiced_buffer_fifo_t* fifos, wiced_buffer_t buffer, wwd_interface_t interface )
{

    wiced_buffer_interface_fifo_t *fifo = &( fifos->per_interface_fifos[ WWD_INTERFACE_INDEX( interface ) ] );
    buffer->next = NULL;

    if ( fifo->first == NULL )
    {
        fifo->first = buffer;
        fifo->last  = buffer;
    }
    else
    {
        fifo->last->next = buffer;
        fifo->last       = buffer;
    }
}

wwd_result_t host_buffer_set_size( /*@temp@*/ wiced_buffer_t buffer, unsigned short size )
{
     if ( size > (unsigned short) WICED_LINK_MTU )
        {
            WPRINT_NETWORK_ERROR(("Attempt to set a length larger than the MTU of the link\n"));
            /*@-unreachable@*/ /* Reachable after hitting assert */
            return WWD_BUFFER_SIZE_SET_ERROR;
            /*@+unreachable@*/
        }
          buffer->tot_len = size;
         buffer->len = size;

         return WWD_SUCCESS;
}

wiced_bool_t host_buffer_pool_is_full( wwd_buffer_dir_t direction )
{
    wiced_bool_t full = WICED_TRUE;
    uint16_t one_byte = 1;
    wiced_buffer_t buffer;

    /* not full if we can pull even one byte long out of the pool */
    if ( WWD_SUCCESS == host_buffer_get( &buffer, direction, one_byte, (wiced_bool_t)WICED_NO_WAIT ) )
    {
        full = WICED_FALSE;
        host_buffer_release( buffer, direction );
    }

    return full;
}

void memp_free_notify( unsigned int type )
{
}

void host_network_send_ethernet_data(char *buff, wwd_interface_t interface)
{
    wiced_buffer_t  buffer = OS_NULL;
    
    buffer = pbuf_alloc(PBUF_RAW, wwd_sdpcm_data_header_size() + ((wiced_buffer_t)buff)->tot_len, PBUF_RAM);
    pbuf_header(buffer, -wwd_sdpcm_data_header_size());
    pbuf_copy(buffer, (wiced_buffer_t)buff);

    wwd_network_send_ethernet_data(buffer, interface);
}
