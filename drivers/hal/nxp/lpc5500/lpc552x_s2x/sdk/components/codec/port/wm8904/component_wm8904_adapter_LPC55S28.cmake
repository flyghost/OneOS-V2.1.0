if(NOT COMPONENT_WM8904_ADAPTER_LPC55S28_INCLUDED)
    
    set(COMPONENT_WM8904_ADAPTER_LPC55S28_INCLUDED true CACHE BOOL "component_wm8904_adapter component is included.")

    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/fsl_codec_adapter.c
    )


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/.
    )


    include(driver_wm8904_LPC55S28)

    include(driver_codec_LPC55S28)

endif()
