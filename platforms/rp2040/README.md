# RP2040 platform

This area is reserved for the current RP2040 Device implementation and its
platform integration. RP2040 support is a current project-status fact, not an
architectural constraint on CEC Injector.

The platform build uses an externally supplied Raspberry Pi Pico SDK through
`PICO_SDK_PATH`. The SDK, TinyUSB, Arm toolchain, and generated files are not
vendored or downloaded by this repository.

The current CMake file establishes the platform boundary only. No firmware
target or production implementation has been migrated yet.
