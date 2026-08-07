# RP2040 platform

This area is reserved for the current RP2040 Device implementation and its
platform integration. RP2040 support is a current project-status fact, not an
architectural constraint on CEC Injector.

The platform build uses an externally supplied Raspberry Pi Pico SDK through
`PICO_SDK_PATH`. The SDK, TinyUSB, Arm toolchain, and generated files are not
vendored or downloaded by this repository.

The electrical platform library uses GP2 for the experimental open-drain CEC
connection by default. Configure `CEC_INJECTOR_RP2040_CEC_GPIO` at CMake time
to select another RP2040 GPIO. This wiring default is a reference-platform
choice, not part of Device Protocol v1.

No complete firmware executable is defined until transport and composition
code are migrated.
