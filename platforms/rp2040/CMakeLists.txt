# SPDX-License-Identifier: Apache-2.0

# The root project validates and initializes an externally supplied Pico SDK
# before entering this directory.
set(CEC_INJECTOR_RP2040_CEC_GPIO 2 CACHE STRING
    "GPIO used for the open-drain CEC line on the RP2040 reference platform")

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
