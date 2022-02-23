if(NOT COMPONENT_SERIAL_MANAGER_LPC55S28_INCLUDED)
    
    set(COMPONENT_SERIAL_MANAGER_LPC55S28_INCLUDED true CACHE BOOL "component_serial_manager component is included.")

    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/fsl_component_serial_manager.c
    )


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/.
    )


    #OR Logic component
    if(CONFIG_USE_component_serial_manager_uart_LPC55S28)
         include(component_serial_manager_uart_LPC55S28)
    endif()
    if(CONFIG_USE_component_serial_manager_usb_cdc_LPC55S28)
         include(component_serial_manager_usb_cdc_LPC55S28)
    endif()
    if(CONFIG_USE_component_serial_manager_virtual_LPC55S28)
         include(component_serial_manager_virtual_LPC55S28)
    endif()
    if(CONFIG_USE_component_serial_manager_swo_LPC55S28)
         include(component_serial_manager_swo_LPC55S28)
    endif()
    if(CONFIG_USE_component_serial_manager_rpmsg_LPC55S28)
         include(component_serial_manager_rpmsg_LPC55S28)
    endif()
    if(NOT (CONFIG_USE_component_serial_manager_uart_LPC55S28 OR CONFIG_USE_component_serial_manager_usb_cdc_LPC55S28 OR CONFIG_USE_component_serial_manager_virtual_LPC55S28 OR CONFIG_USE_component_serial_manager_swo_LPC55S28 OR CONFIG_USE_component_serial_manager_rpmsg_LPC55S28))
        message(WARNING "Since component_serial_manager_uart_LPC55S28/component_serial_manager_usb_cdc_LPC55S28/component_serial_manager_virtual_LPC55S28/component_serial_manager_swo_LPC55S28/component_serial_manager_rpmsg_LPC55S28 is not included at first or config in config.cmake file, use component_serial_manager_uart_LPC55S28 by default.")
        include(component_serial_manager_uart_LPC55S28)
    endif()

    include(driver_common_LPC55S28)

    include(component_lists_LPC55S28)

endif()
