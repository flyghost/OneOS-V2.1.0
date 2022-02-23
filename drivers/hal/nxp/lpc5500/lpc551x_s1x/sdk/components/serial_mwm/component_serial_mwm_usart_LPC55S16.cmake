if(NOT COMPONENT_SERIAL_MWM_USART_LPC55S16_INCLUDED)
    
    set(COMPONENT_SERIAL_MWM_USART_LPC55S16_INCLUDED true CACHE BOOL "component_serial_mwm_usart component is included.")

    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/serial_mwm_usart.c
    )


    include(driver_flexcomm_usart_freertos_LPC55S16)

endif()
