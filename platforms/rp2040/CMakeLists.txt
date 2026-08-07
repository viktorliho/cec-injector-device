# SPDX-License-Identifier: Apache-2.0

# The root project validates and initializes an externally supplied Pico SDK
# before entering this directory.
set(CEC_INJECTOR_RP2040_CEC_GPIO 2 CACHE STRING
    "GPIO used for the open-drain CEC line on the RP2040 reference platform")
set(CEC_INJECTOR_USB_VID "0xCAFE" CACHE STRING
    "Development USB vendor identifier for the RP2040 reference build")
set(CEC_INJECTOR_USB_PID "0xCEC1" CACHE STRING
    "Development USB product identifier for the RP2040 reference build")
set(CEC_INJECTOR_USB_MANUFACTURER "CEC Injector Project" CACHE STRING
    "USB manufacturer string for the RP2040 reference build")
set(CEC_INJECTOR_USB_PRODUCT "RP2040 HDMI-CEC Adapter (Development)" CACHE STRING
    "USB product string for the RP2040 reference build")

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/usb_identity.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/usb_identity.h
    @ONLY
)

add_library(cec_injector_rp2040_electrical STATIC
    rp2040_cec_analyzer.c
    rp2040_cec_follower_ack.c
    rp2040_cec_gpio.c
    rp2040_cec_tx.c
)
add_library(CECInjector::RP2040Electrical ALIAS
    cec_injector_rp2040_electrical)

target_include_directories(cec_injector_rp2040_electrical PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_compile_definitions(cec_injector_rp2040_electrical PUBLIC
    CEC_INJECTOR_RP2040_CEC_GPIO=${CEC_INJECTOR_RP2040_CEC_GPIO}
)
target_compile_features(cec_injector_rp2040_electrical PUBLIC c_std_11)
target_compile_options(cec_injector_rp2040_electrical PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
target_link_libraries(cec_injector_rp2040_electrical PUBLIC
    CECInjector::DeviceCore
    pico_stdlib
    hardware_gpio
    hardware_sync
    hardware_timer
)

add_library(cec_injector_rp2040_usb STATIC
    rp2040_usb_lifecycle.c
    rp2040_usb_remote_wake.c
    rp2040_usb_tinyusb.c
    rp2040_usb_transport.c
    usb_descriptors.c
)
add_library(CECInjector::RP2040USB ALIAS cec_injector_rp2040_usb)
target_include_directories(cec_injector_rp2040_usb PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/generated
)
target_compile_features(cec_injector_rp2040_usb PUBLIC c_std_11)
target_compile_options(cec_injector_rp2040_usb PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
target_compile_definitions(cec_injector_rp2040_usb PUBLIC
    PICO_STDIO_USB_ENABLE_RESET_VIA_VENDOR_INTERFACE=0
    PICO_STDIO_USB_USE_DEFAULT_DESCRIPTORS=0
)
target_link_libraries(cec_injector_rp2040_usb PUBLIC
    CECInjector::DeviceCore
    pico_stdlib
)
