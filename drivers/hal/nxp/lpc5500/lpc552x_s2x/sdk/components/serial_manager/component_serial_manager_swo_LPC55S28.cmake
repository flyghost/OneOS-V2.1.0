if(NOT COMPONENT_SERIAL_MANAGER_SWO_LPC55S28_INCLUDED)
    
    set(COMPONENT_SERIAL_MANAGER_SWO_LPC55S28_INCLUDED true CACHE BOOL "component_serial_manager_swo component is included.")

    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/fsl_component_serial_port_swo.c
    )


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/.
    )


    include(driver_common_LPC55S28)

    include(component_serial_manager_LPC55S28)

endif()
