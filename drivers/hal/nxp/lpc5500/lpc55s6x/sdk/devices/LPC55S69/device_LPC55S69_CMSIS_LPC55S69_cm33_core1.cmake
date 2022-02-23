include_guard(GLOBAL)
message("device_LPC55S69_CMSIS component is included.")


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/.
)

include(CMSIS_Include_core_cm_LPC55S69_cm33_core1)

