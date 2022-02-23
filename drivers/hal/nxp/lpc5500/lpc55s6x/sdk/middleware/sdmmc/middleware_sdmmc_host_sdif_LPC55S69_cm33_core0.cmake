include_guard(GLOBAL)
message("middleware_sdmmc_host_sdif component is included.")


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/host/sdif
)

#OR Logic component
if(CONFIG_USE_middleware_sdmmc_host_sdif_freertos_LPC55S69_cm33_core0)
     include(middleware_sdmmc_host_sdif_freertos_LPC55S69_cm33_core0)
endif()
if(CONFIG_USE_middleware_sdmmc_host_sdif_interrupt_LPC55S69_cm33_core0)
     include(middleware_sdmmc_host_sdif_interrupt_LPC55S69_cm33_core0)
endif()
if(CONFIG_USE_middleware_sdmmc_host_sdif_polling_LPC55S69_cm33_core0)
     include(middleware_sdmmc_host_sdif_polling_LPC55S69_cm33_core0)
endif()
if(NOT (CONFIG_USE_middleware_sdmmc_host_sdif_freertos_LPC55S69_cm33_core0 OR CONFIG_USE_middleware_sdmmc_host_sdif_interrupt_LPC55S69_cm33_core0 OR CONFIG_USE_middleware_sdmmc_host_sdif_polling_LPC55S69_cm33_core0))
    message(WARNING "Since middleware_sdmmc_host_sdif_freertos_LPC55S69_cm33_core0/middleware_sdmmc_host_sdif_interrupt_LPC55S69_cm33_core0/middleware_sdmmc_host_sdif_polling_LPC55S69_cm33_core0 is not included at first or config in config.cmake file, use middleware_sdmmc_host_sdif_interrupt_LPC55S69_cm33_core0 by default.")
    include(middleware_sdmmc_host_sdif_interrupt_LPC55S69_cm33_core0)
endif()

