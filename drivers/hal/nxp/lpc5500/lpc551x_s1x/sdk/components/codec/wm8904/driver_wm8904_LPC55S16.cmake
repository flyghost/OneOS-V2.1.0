if(NOT DRIVER_WM8904_LPC55S16_INCLUDED)
    
    set(DRIVER_WM8904_LPC55S16_INCLUDED true CACHE BOOL "driver_wm8904 component is included.")

    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/fsl_wm8904.c
    )


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/.
    )


    include(driver_common_LPC55S16)

    include(component_codec_i2c_LPC55S16)

endif()
