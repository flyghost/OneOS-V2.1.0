include_guard(GLOBAL)
message("middleware_usb_device_common_header component is included.")


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/device
)

include(component_osa_LPC55S69_cm33_core0)

include(middleware_usb_common_header_LPC55S69_cm33_core0)

