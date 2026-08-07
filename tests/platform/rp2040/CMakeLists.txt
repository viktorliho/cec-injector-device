# SPDX-License-Identifier: Apache-2.0

add_executable(cec_injector_rp2040_usb_transport_test
    test_usb_transport.c
    ${PROJECT_SOURCE_DIR}/platforms/rp2040/rp2040_usb_transport.c
)
target_link_libraries(cec_injector_rp2040_usb_transport_test PRIVATE
    CECInjector::DeviceCore
)
target_include_directories(cec_injector_rp2040_usb_transport_test PRIVATE
    ${PROJECT_SOURCE_DIR}/platforms/rp2040
)
target_compile_features(cec_injector_rp2040_usb_transport_test PRIVATE c_std_11)
target_compile_options(cec_injector_rp2040_usb_transport_test PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
)
add_test(NAME platform.rp2040_usb_transport
    COMMAND cec_injector_rp2040_usb_transport_test)

add_executable(cec_injector_rp2040_usb_remote_wake_test
    test_usb_remote_wake.c
    ${PROJECT_SOURCE_DIR}/platforms/rp2040/rp2040_usb_lifecycle.c
    ${PROJECT_SOURCE_DIR}/platforms/rp2040/rp2040_usb_remote_wake.c
    ${PROJECT_SOURCE_DIR}/platforms/rp2040/rp2040_usb_transport.c
)
target_link_libraries(cec_injector_rp2040_usb_remote_wake_test PRIVATE
    CECInjector::DeviceCore
)
target_include_directories(cec_injector_rp2040_usb_remote_wake_test PRIVATE
    ${PROJECT_SOURCE_DIR}/platforms/rp2040
    ${PROJECT_SOURCE_DIR}/tests/stubs/rp2040
)
target_compile_features(cec_injector_rp2040_usb_remote_wake_test PRIVATE c_std_11)
target_compile_options(cec_injector_rp2040_usb_remote_wake_test PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
)
add_test(NAME platform.rp2040_usb_remote_wake
    COMMAND cec_injector_rp2040_usb_remote_wake_test)
