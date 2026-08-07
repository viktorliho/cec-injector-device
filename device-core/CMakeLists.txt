# SPDX-License-Identifier: Apache-2.0

# DeviceCore must remain buildable with an ordinary host C compiler.
add_library(cec_injector_device_core STATIC
    adapter_state.c
    cec_follower_ack.c
    cec_rx.c
    cec_transport.c
    pulse8_profile.c
    pulse8_serial.c
    wake_extension.c
    wake_extension_codec.c
)
add_library(CECInjector::DeviceCore ALIAS cec_injector_device_core)

target_include_directories(cec_injector_device_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_compile_features(cec_injector_device_core PUBLIC c_std_11)
target_compile_options(cec_injector_device_core PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

if(CEC_INJECTOR_BUILD_TESTS)
    target_compile_definitions(cec_injector_device_core PRIVATE
        CEC_INJECTOR_TESTING
    )
endif()
