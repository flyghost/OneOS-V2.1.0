if(NOT MIDDLEWARE_USB_DEVICE_IP3511FS_LPC55S28_INCLUDED)
    
    set(MIDDLEWARE_USB_DEVICE_IP3511FS_LPC55S28_INCLUDED true CACHE BOOL "middleware_usb_device_ip3511fs component is included.")

    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/device/usb_device_lpcip3511.c
    )


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/device
        ${CMAKE_CURRENT_LIST_DIR}/include
    )


    include(middleware_usb_device_common_header_LPC55S28)

endif()
