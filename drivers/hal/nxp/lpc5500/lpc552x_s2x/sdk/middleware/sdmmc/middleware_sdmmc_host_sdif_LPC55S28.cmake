if(NOT MIDDLEWARE_SDMMC_HOST_SDIF_LPC55S28_INCLUDED)
    
    set(MIDDLEWARE_SDMMC_HOST_SDIF_LPC55S28_INCLUDED true CACHE BOOL "middleware_sdmmc_host_sdif component is included.")


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/host/sdif
    )

    #OR Logic component
    if(CONFIG_USE_middleware_sdmmc_host_sdif_freertos_LPC55S28)
         include(middleware_sdmmc_host_sdif_freertos_LPC55S28)
    endif()
    if(CONFIG_USE_middleware_sdmmc_host_sdif_interrupt_LPC55S28)
         include(middleware_sdmmc_host_sdif_interrupt_LPC55S28)
    endif()
    if(CONFIG_USE_middleware_sdmmc_host_sdif_polling_LPC55S28)
         include(middleware_sdmmc_host_sdif_polling_LPC55S28)
    endif()
    if(NOT (CONFIG_USE_middleware_sdmmc_host_sdif_freertos_LPC55S28 OR CONFIG_USE_middleware_sdmmc_host_sdif_interrupt_LPC55S28 OR CONFIG_USE_middleware_sdmmc_host_sdif_polling_LPC55S28))
        message(WARNING "Since middleware_sdmmc_host_sdif_freertos_LPC55S28/middleware_sdmmc_host_sdif_interrupt_LPC55S28/middleware_sdmmc_host_sdif_polling_LPC55S28 is not included at first or config in config.cmake file, use middleware_sdmmc_host_sdif_interrupt_LPC55S28 by default.")
        include(middleware_sdmmc_host_sdif_interrupt_LPC55S28)
    endif()

endif()
