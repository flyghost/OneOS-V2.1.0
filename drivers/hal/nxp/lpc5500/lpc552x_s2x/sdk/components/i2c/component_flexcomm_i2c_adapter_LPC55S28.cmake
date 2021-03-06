if(NOT COMPONENT_FLEXCOMM_I2C_ADAPTER_LPC55S28_INCLUDED)
    
    set(COMPONENT_FLEXCOMM_I2C_ADAPTER_LPC55S28_INCLUDED true CACHE BOOL "component_flexcomm_i2c_adapter component is included.")

    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/fsl_adapter_flexcomm_i2c.c
    )


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/.
    )


    include(driver_common_LPC55S28)

    include(driver_flexcomm_i2c_LPC55S28)

endif()
