if(NOT MIDDLEWARE_SDMMC_HOST_SDIF_POLLING_LPC55S28_INCLUDED)
    
    set(MIDDLEWARE_SDMMC_HOST_SDIF_POLLING_LPC55S28_INCLUDED true CACHE BOOL "middleware_sdmmc_host_sdif_polling component is included.")

    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/host/sdif/blocking/fsl_sdmmc_host.c
    )


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/host/sdif
    )


    include(middleware_sdmmc_common_LPC55S28)

    include(middleware_sdmmc_osa_bm_LPC55S28)

    include(driver_sdif_LPC55S28)

endif()
