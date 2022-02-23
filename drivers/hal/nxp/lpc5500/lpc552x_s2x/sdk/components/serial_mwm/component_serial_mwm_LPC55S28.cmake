if(NOT COMPONENT_SERIAL_MWM_LPC55S28_INCLUDED)
    
    set(COMPONENT_SERIAL_MWM_LPC55S28_INCLUDED true CACHE BOOL "component_serial_mwm component is included.")

    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/serial_mwm.c
    )


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/.
    )


    #OR Logic component
    if(CONFIG_USE_component_serial_mwm_usart_LPC55S28)
         include(component_serial_mwm_usart_LPC55S28)
    endif()
    if(CONFIG_USE_component_serial_mwm_lpuart_LPC55S28)
         include(component_serial_mwm_lpuart_LPC55S28)
    endif()
    if(NOT (CONFIG_USE_component_serial_mwm_usart_LPC55S28 OR CONFIG_USE_component_serial_mwm_lpuart_LPC55S28))
        message(WARNING "Since component_serial_mwm_usart_LPC55S28/component_serial_mwm_lpuart_LPC55S28 is not included at first or config in config.cmake file, use component_serial_mwm_usart_LPC55S28 by default.")
        include(component_serial_mwm_usart_LPC55S28)
    endif()

endif()
