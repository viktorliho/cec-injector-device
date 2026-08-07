# SPDX-License-Identifier: Apache-2.0

add_executable(cec_injector_adapter_state_test test_adapter_state.c)
target_link_libraries(cec_injector_adapter_state_test PRIVATE
    CECInjector::DeviceCore
)
target_compile_options(cec_injector_adapter_state_test PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

add_test(NAME core.adapter_state
    COMMAND cec_injector_adapter_state_test
)
