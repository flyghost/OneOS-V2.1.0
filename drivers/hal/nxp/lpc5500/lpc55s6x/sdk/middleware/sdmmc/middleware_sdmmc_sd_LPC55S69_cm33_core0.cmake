include_guard(GLOBAL)
message("middleware_sdmmc_sd component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/sd/fsl_sd.c
)


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/sd
)


#OR Logic component
if(CONFIG_USE_middleware_sdmmc_host_sdhc_LPC55S69_cm33_core0)
     include(middleware_sdmmc_host_sdhc_LPC55S69_cm33_core0)
endif()
if(CONFIG_USE_middleware_sdmmc_host_usdhc_LPC55S69_cm33_core0)
     include(middleware_sdmmc_host_usdhc_LPC55S69_cm33_core0)
endif()
if(CONFIG_USE_middleware_sdmmc_host_sdif_LPC55S69_cm33_core0)
     include(middleware_sdmmc_host_sdif_LPC55S69_cm33_core0)
endif()
if(NOT (CONFIG_USE_middleware_sdmmc_host_sdhc_LPC55S69_cm33_core0 OR CONFIG_USE_middleware_sdmmc_host_usdhc_LPC55S69_cm33_core0 OR CONFIG_USE_middleware_sdmmc_host_sdif_LPC55S69_cm33_core0))
    message(WARNING "Since middleware_sdmmc_host_sdhc_LPC55S69_cm33_core0/middleware_sdmmc_host_usdhc_LPC55S69_cm33_core0/middleware_sdmmc_host_sdif_LPC55S69_cm33_core0 is not included at first or config in config.cmake file, use middleware_sdmmc_host_usdhc_LPC55S69_cm33_core0 by default.")
    include(middleware_sdmmc_host_usdhc_LPC55S69_cm33_core0)
endif()

include(middleware_sdmmc_common_LPC55S69_cm33_core0)

