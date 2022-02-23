include_guard(GLOBAL)
message("driver_clock component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/fsl_clock.c
)


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/.
)


#OR Logic component
if(CONFIG_USE_driver_power_LPC55S69_cm33_core1)
     include(driver_power_LPC55S69_cm33_core1)
endif()
if(CONFIG_USE_driver_power_s_LPC55S69_cm33_core1)
     include(driver_power_s_LPC55S69_cm33_core1)
endif()
if(NOT (CONFIG_USE_driver_power_LPC55S69_cm33_core1 OR CONFIG_USE_driver_power_s_LPC55S69_cm33_core1))
    message(WARNING "Since driver_power_LPC55S69_cm33_core1/driver_power_s_LPC55S69_cm33_core1 is not included at first or config in config.cmake file, use driver_power_LPC55S69_cm33_core1 by default.")
    include(driver_power_LPC55S69_cm33_core1)
endif()

include(driver_common_LPC55S69_cm33_core1)

