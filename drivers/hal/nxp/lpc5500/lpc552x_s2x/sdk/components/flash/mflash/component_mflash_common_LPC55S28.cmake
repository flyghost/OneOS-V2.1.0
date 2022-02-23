if(NOT COMPONENT_MFLASH_COMMON_LPC55S28_INCLUDED)
    
    set(COMPONENT_MFLASH_COMMON_LPC55S28_INCLUDED true CACHE BOOL "component_mflash_common component is included.")


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/.
    )

endif()
