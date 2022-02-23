if(NOT MIDDLEWARE_USB_HOST_COMMON_HEADER_LPC55S28_INCLUDED)
    
    set(MIDDLEWARE_USB_HOST_COMMON_HEADER_LPC55S28_INCLUDED true CACHE BOOL "middleware_usb_host_common_header component is included.")


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/host
    )

    include(component_osa_LPC55S28)

    include(middleware_usb_common_header_LPC55S28)

endif()
