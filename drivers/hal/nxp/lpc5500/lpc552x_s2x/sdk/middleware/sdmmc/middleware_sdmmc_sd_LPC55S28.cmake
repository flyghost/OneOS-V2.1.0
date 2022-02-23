if(NOT MIDDLEWARE_SDMMC_SD_LPC55S28_INCLUDED)
    
    set(MIDDLEWARE_SDMMC_SD_LPC55S28_INCLUDED true CACHE BOOL "middleware_sdmmc_sd component is included.")

    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/sd/fsl_sd.c
    )


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/sd
    )


    #OR Logic component
    if(CONFIG_USE_middleware_sdmmc_host_sdhc_LPC55S28)
         include(middleware_sdmmc_host_sdhc_LPC55S28)
    endif()
    if(CONFIG_USE_middleware_sdmmc_host_usdhc_LPC55S28)
         include(middleware_sdmmc_host_usdhc_LPC55S28)
    endif()
    if(CONFIG_USE_middleware_sdmmc_host_sdif_LPC55S28)
         include(middleware_sdmmc_host_sdif_LPC55S28)
    endif()
    if(NOT (CONFIG_USE_middleware_sdmmc_host_sdhc_LPC55S28 OR CONFIG_USE_middleware_sdmmc_host_usdhc_LPC55S28 OR CONFIG_USE_middleware_sdmmc_host_sdif_LPC55S28))
        message(WARNING "Since middleware_sdmmc_host_sdhc_LPC55S28/middleware_sdmmc_host_usdhc_LPC55S28/middleware_sdmmc_host_sdif_LPC55S28 is not included at first or config in config.cmake file, use middleware_sdmmc_host_usdhc_LPC55S28 by default.")
        include(middleware_sdmmc_host_usdhc_LPC55S28)
    endif()

    include(middleware_sdmmc_common_LPC55S28)

endif()
