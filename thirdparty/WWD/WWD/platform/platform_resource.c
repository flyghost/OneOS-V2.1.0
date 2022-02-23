/*
 * Broadcom Proprietary and Confidential. Copyright 2016 Broadcom
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/** @file
 * Common resource implementation
 */

#include <stdio.h>
#include <string.h>
#include "stdlib.h"
//#include "wicedfs.h"
#include "wiced_resource.h"
#include "platform_resource.h"
#include "platform_toolchain.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Static Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

resource_result_t resource_read ( const resource_hnd_t* resource, uint32_t offset, uint32_t maxsize, uint32_t* size, void* buffer )
{
    if ( offset > resource->size )
    {
        return RESOURCE_OFFSET_TOO_BIG;
    }

    *size = MIN( maxsize, resource->size - offset );

    if (resource->location == RESOURCE_IN_MEMORY)
    {
        memcpy(buffer, &resource->val.mem.data[offset], *size);
    }
#ifdef USES_RESOURCES_IN_EXTERNAL_STORAGE
    else if ( resource->location == RESOURCE_IN_EXTERNAL_STORAGE )
    {
        return platform_read_external_resource( resource, offset, maxsize, size, buffer );
    }
#endif
#ifdef USES_RESOURCE_FILESYSTEM
    else
    {
        wicedfs_file_t file_hnd;

        if ( 0 != wicedfs_fopen( &resource_fs_handle, &file_hnd, resource->val.fs.filename ) )
        {
            return RESOURCE_FILE_OPEN_FAIL;
        }

        if ( 0 != wicedfs_fseek( &file_hnd, (long)(offset + resource->val.fs.offset), SEEK_SET ) )
        {
            wicedfs_fclose( &file_hnd );
            return RESOURCE_FILE_SEEK_FAIL;
        }

        if ( *size != wicedfs_fread( buffer, 1, *size, &file_hnd ) )
        {
            wicedfs_fclose( &file_hnd );
            return RESOURCE_FILE_READ_FAIL;
        }

        wicedfs_fclose( &file_hnd );
    }
#endif /* ifdef USES_RESOURCE_FILESYSTEM */
    return RESOURCE_SUCCESS;
}

resource_result_t resource_get_readonly_buffer ( const resource_hnd_t* resource, uint32_t offset, uint32_t maxsize, uint32_t* size_out, const void** buffer )
{
    UNUSED_PARAMETER( maxsize );

    if ( offset > resource->size )
    {
        return RESOURCE_OFFSET_TOO_BIG;
    }

    if ( resource->location == RESOURCE_IN_MEMORY )
    {
        *buffer   = &resource->val.mem.data[offset];
        *size_out = MIN(maxsize, resource->size - offset);
        return RESOURCE_SUCCESS;
    }
#ifdef USES_RESOURCE_FILESYSTEM
    else
    {
        resource_result_t result;
        uint32_t size_returned = 0;
        *size_out = MIN(maxsize, resource->size - offset);
        *buffer = malloc( *size_out );
        result = resource_read( resource, offset, *size_out, &size_returned, (void*) *buffer );
        *size_out = MIN( *size_out, size_returned );

        if ( result != RESOURCE_SUCCESS )
        {
            free( (void*) *buffer );
            return result;
        }
        return RESOURCE_SUCCESS;
    }
#endif /* ifdef USES_RESOURCE_FILESYSTEM */
    return RESOURCE_UNSUPPORTED;
}

resource_result_t resource_free_readonly_buffer( const resource_hnd_t* resource, const void* buffer )
{
    if ( resource->location != RESOURCE_IN_MEMORY )
    {
        free( (void*) buffer );
    }
    return RESOURCE_SUCCESS;
}

