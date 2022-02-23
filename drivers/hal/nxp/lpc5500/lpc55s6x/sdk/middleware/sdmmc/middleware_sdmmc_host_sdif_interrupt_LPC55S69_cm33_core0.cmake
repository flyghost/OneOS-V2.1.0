include_guard(GLOBAL)
message("middleware_sdmmc_host_sdif_interrupt component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/host/sdif/non_blocking/fsl_sdmmc_host.c
)


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/host/sdif
)


include(middleware_sdmmc_common_LPC55S69_cm33_core0)

include(middleware_sdmmc_osa_bm_LPC55S69_cm33_core0)

include(driver_sdif_LPC55S69_cm33_core0)

