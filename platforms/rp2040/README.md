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

The reference USB carrier defaults to the development identity `CAFE:CEC1`.
`CEC_INJECTOR_USB_VID`, `CEC_INJECTOR_USB_PID`, and the manufacturer/product
strings are build-time settings. These values identify a reference build, not
Device Protocol v1, and must be replaced with legitimately assigned values for
distribution where required.

The single CDC ACM function has two interfaces and no HID function. Its serial
string is generated from the RP2040 unique board identifier; no fixed board
serial is stored in the repository. USB suspend preserves the transport
attachment session. Final firmware composition will connect TinyUSB lifecycle
callbacks and Remote Wake delivery in later migration phases.
