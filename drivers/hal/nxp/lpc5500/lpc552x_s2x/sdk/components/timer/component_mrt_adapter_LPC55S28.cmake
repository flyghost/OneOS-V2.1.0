if(NOT COMPONENT_MRT_ADAPTER_LPC55S28_INCLUDED)
    
    set(COMPONENT_MRT_ADAPTER_LPC55S28_INCLUDED true CACHE BOOL "component_mrt_adapter component is included.")

    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/fsl_adapter_mrt.c
    )


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/.
    )


    include(driver_common_LPC55S28)

    include(driver_mrt_LPC55S28)

endif()
