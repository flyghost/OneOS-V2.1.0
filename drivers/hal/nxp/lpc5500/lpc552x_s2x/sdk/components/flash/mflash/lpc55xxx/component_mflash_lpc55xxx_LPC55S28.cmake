if(NOT COMPONENT_MFLASH_LPC55XXX_LPC55S28_INCLUDED)
    
    set(COMPONENT_MFLASH_LPC55XXX_LPC55S28_INCLUDED true CACHE BOOL "component_mflash_lpc55xxx component is included.")

    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/mflash_drv.c
    )


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/.
    )


    include(component_mflash_common_LPC55S28)

    include(driver_iap1_LPC55S28)

endif()
