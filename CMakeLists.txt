# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.25)

option(CEC_INJECTOR_BUILD_TESTS
    "Build host-side CEC Injector tests" ON)
option(CEC_INJECTOR_BUILD_RP2040
    "Configure the RP2040 reference platform" OFF)

if(CEC_INJECTOR_BUILD_RP2040)
    if(DEFINED PICO_SDK_PATH)
        set(_cec_injector_pico_sdk_path "${PICO_SDK_PATH}")
    elseif(DEFINED ENV{PICO_SDK_PATH})
        set(_cec_injector_pico_sdk_path "$ENV{PICO_SDK_PATH}")
    else()
        message(FATAL_ERROR
            "CEC_INJECTOR_BUILD_RP2040 requires an external PICO_SDK_PATH")
    endif()

    set(_cec_injector_pico_sdk_import
        "${_cec_injector_pico_sdk_path}/external/pico_sdk_import.cmake")
    if(NOT EXISTS "${_cec_injector_pico_sdk_import}")
        message(FATAL_ERROR
            "Pico SDK import file not found: ${_cec_injector_pico_sdk_import}")
    endif()
    include("${_cec_injector_pico_sdk_import}")
endif()

if(CEC_INJECTOR_BUILD_RP2040)
    project(cec_injector_device LANGUAGES C CXX ASM)
else()
    project(cec_injector_device LANGUAGES C)
endif()

add_subdirectory(device-core)

if(CEC_INJECTOR_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(CEC_INJECTOR_BUILD_RP2040)
    pico_sdk_init()
    add_subdirectory(platforms/rp2040)
endif()
