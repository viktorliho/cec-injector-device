# SPDX-License-Identifier: Apache-2.0

add_executable(cec_injector_serial_binding_test test_pulse8_serial.c)
target_link_libraries(cec_injector_serial_binding_test PRIVATE
    CECInjector::DeviceCore
)
target_compile_options(cec_injector_serial_binding_test PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

add_test(NAME conformance.pulse8_serial_binding
    COMMAND cec_injector_serial_binding_test
)

add_executable(cec_injector_pulse8_profile_test test_pulse8_profile.c)
target_link_libraries(cec_injector_pulse8_profile_test PRIVATE
    CECInjector::DeviceCore
)
target_compile_options(cec_injector_pulse8_profile_test PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

add_test(NAME conformance.pulse8_compatibility_profile
    COMMAND cec_injector_pulse8_profile_test
)
