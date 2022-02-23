if(NOT MIDDLEWARE_USB_HOST_AUDIO_LPC55S28_INCLUDED)
    
    set(MIDDLEWARE_USB_HOST_AUDIO_LPC55S28_INCLUDED true CACHE BOOL "middleware_usb_host_audio component is included.")

    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/host/class/usb_host_audio.c
    )


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/host/class
    )


    include(middleware_usb_host_stack_LPC55S28)

endif()
