if(NOT COMPONENT_MFLASH_FILE_LPC55S28_INCLUDED)
    
    set(COMPONENT_MFLASH_FILE_LPC55S28_INCLUDED true CACHE BOOL "component_mflash_file component is included.")

    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/mflash_file.c
    )


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/.
    )


    #OR Logic component
    if(CONFIG_USE_component_mflash_frdmk64f_LPC55S28)
         include(component_mflash_frdmk64f_LPC55S28)
    endif()
    if(CONFIG_USE_component_mflash_frdmk66f_LPC55S28)
         include(component_mflash_frdmk66f_LPC55S28)
    endif()
    if(CONFIG_USE_component_mflash_frdmk82f_LPC55S28)
         include(component_mflash_frdmk82f_LPC55S28)
    endif()
    if(CONFIG_USE_component_mflash_lpc54s018m_LPC55S28)
         include(component_mflash_lpc54s018m_LPC55S28)
    endif()
    if(CONFIG_USE_component_mflash_lpc54xxx_LPC55S28)
         include(component_mflash_lpc54xxx_LPC55S28)
    endif()
    if(CONFIG_USE_component_mflash_lpc55xxx_LPC55S28)
         include(component_mflash_lpc55xxx_LPC55S28)
    endif()
    if(CONFIG_USE_component_mflash_rt1020_LPC55S28)
         include(component_mflash_rt1020_LPC55S28)
    endif()
    if(CONFIG_USE_component_mflash_rt1024_LPC55S28)
         include(component_mflash_rt1024_LPC55S28)
    endif()
    if(CONFIG_USE_component_mflash_rt1050_LPC55S28)
         include(component_mflash_rt1050_LPC55S28)
    endif()
    if(CONFIG_USE_component_mflash_rt1060_LPC55S28)
         include(component_mflash_rt1060_LPC55S28)
    endif()
    if(CONFIG_USE_component_mflash_rt1064_LPC55S28)
         include(component_mflash_rt1064_LPC55S28)
    endif()
    if(CONFIG_USE_component_mflash_rt1170_LPC55S28)
         include(component_mflash_rt1170_LPC55S28)
    endif()
    if(CONFIG_USE_component_mflash_rt685_LPC55S28)
         include(component_mflash_rt685_LPC55S28)
    endif()
    if(CONFIG_USE_component_mflash_rt595_LPC55S28)
         include(component_mflash_rt595_LPC55S28)
    endif()
    if(NOT (CONFIG_USE_component_mflash_frdmk64f_LPC55S28 OR CONFIG_USE_component_mflash_frdmk66f_LPC55S28 OR CONFIG_USE_component_mflash_frdmk82f_LPC55S28 OR CONFIG_USE_component_mflash_lpc54s018m_LPC55S28 OR CONFIG_USE_component_mflash_lpc54xxx_LPC55S28 OR CONFIG_USE_component_mflash_lpc55xxx_LPC55S28 OR CONFIG_USE_component_mflash_rt1020_LPC55S28 OR CONFIG_USE_component_mflash_rt1024_LPC55S28 OR CONFIG_USE_component_mflash_rt1050_LPC55S28 OR CONFIG_USE_component_mflash_rt1060_LPC55S28 OR CONFIG_USE_component_mflash_rt1064_LPC55S28 OR CONFIG_USE_component_mflash_rt1170_LPC55S28 OR CONFIG_USE_component_mflash_rt685_LPC55S28 OR CONFIG_USE_component_mflash_rt595_LPC55S28))
        message(WARNING "Since component_mflash_frdmk64f_LPC55S28/component_mflash_frdmk66f_LPC55S28/component_mflash_frdmk82f_LPC55S28/component_mflash_lpc54s018m_LPC55S28/component_mflash_lpc54xxx_LPC55S28/component_mflash_lpc55xxx_LPC55S28/component_mflash_rt1020_LPC55S28/component_mflash_rt1024_LPC55S28/component_mflash_rt1050_LPC55S28/component_mflash_rt1060_LPC55S28/component_mflash_rt1064_LPC55S28/component_mflash_rt1170_LPC55S28/component_mflash_rt685_LPC55S28/component_mflash_rt595_LPC55S28 is not included at first or config in config.cmake file, use component_mflash_frdmk64f_LPC55S28 by default.")
        include(component_mflash_frdmk64f_LPC55S28)
    endif()

endif()
