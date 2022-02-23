if(NOT DRIVER_I2C_EDMA_MK22F51212_INCLUDED)
    
    set(DRIVER_I2C_EDMA_MK22F51212_INCLUDED true CACHE BOOL "driver_i2c_edma component is included.")

    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/fsl_i2c_edma.c
    )


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/.
    )


    include(driver_i2c_MK22F51212)

    include(driver_edma_MK22F51212)

endif()
