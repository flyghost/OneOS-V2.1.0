if(NOT COMPONENT_SERIAL_MANAGER_LPC55S16_INCLUDED)
    
    set(COMPONENT_SERIAL_MANAGER_LPC55S16_INCLUDED true CACHE BOOL "component_serial_manager component is included.")

    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/fsl_component_serial_manager.c
    )


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/.
    )


    #OR Logic component
    if(CONFIG_USE_component_serial_manager_uart_LPC55S16)
         include(component_serial_manager_uart_LPC55S16)
    endif()
    if(CONFIG_USE_component_serial_manager_usb_cdc_LPC55S16)
         include(component_serial_manager_usb_cdc_LPC55S16)
    endif()
    if(CONFIG_USE_component_serial_manager_virtual_LPC55S16)
         include(component_serial_manager_virtual_LPC55S16)
    endif()
    if(CONFIG_USE_component_serial_manager_swo_LPC55S16)
         include(component_serial_manager_swo_LPC55S16)
    endif()
    if(CONFIG_USE_component_serial_manager_rpmsg_LPC55S16)
         include(component_serial_manager_rpmsg_LPC55S16)
    endif()
    if(NOT (CONFIG_USE_component_serial_manager_uart_LPC55S16 OR CONFIG_USE_component_serial_manager_usb_cdc_LPC55S16 OR CONFIG_USE_component_serial_manager_virtual_LPC55S16 OR CONFIG_USE_component_serial_manager_swo_LPC55S16 OR CONFIG_USE_component_serial_manager_rpmsg_LPC55S16))
        message(WARNING "Since component_serial_manager_uart_LPC55S16/component_serial_manager_usb_cdc_LPC55S16/component_serial_manager_virtual_LPC55S16/component_serial_manager_swo_LPC55S16/component_serial_manager_rpmsg_LPC55S16 is not included at first or config in config.cmake file, use component_serial_manager_uart_LPC55S16 by default.")
        include(component_serial_manager_uart_LPC55S16)
    endif()

    include(driver_common_LPC55S16)

    include(component_lists_LPC55S16)

endif()
