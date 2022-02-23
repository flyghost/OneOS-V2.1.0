if(NOT COMPONENT_GINT_ADAPTER_LPC55S28_INCLUDED)
    
    set(COMPONENT_GINT_ADAPTER_LPC55S28_INCLUDED true CACHE BOOL "component_gint_adapter component is included.")

    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/fsl_adapter_gint.c
    )


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/.
    )


    include(driver_gint_LPC55S28)

endif()
