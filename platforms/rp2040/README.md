# RP2040 platform

This directory contains the production RP2040 reference firmware and its
platform integration. It provides the GPIO/open-drain CEC driver, electrical
observation analyzer, physical CEC transmitter, follower-ACK actuator, USB CDC
transport, attachment and suspend lifecycle handling, USB Remote Wake binding,
and production composition.

RP2040 support is a current project-status fact, not an architectural
constraint on CEC Injector.

The platform build uses an externally supplied Raspberry Pi Pico SDK through
`PICO_SDK_PATH`. The SDK, TinyUSB, Arm toolchain, and generated files are not
vendored or downloaded by this repository.

The electrical platform library uses GP2 for the experimental open-drain CEC
connection by default. Configure `CEC_INJECTOR_RP2040_CEC_GPIO` at CMake time
to select another RP2040 GPIO. This wiring default is a reference-platform
choice, not part of Device Protocol v1.

The Experimental production reference target is `cec_adapter`. It composes
DeviceCore with the RP2040 electrical backend, CDC transport, attachment
lifecycle, and USB Remote Wake binding. It does not include debug commands or
suspend-capture tooling.

The protocol transport owns TinyUSB initialization, cooperative `tud_task()`
servicing, CDC RX/TX, and lifecycle callbacks. Pico USB stdio is disabled so
stdin/stdout cannot consume or inject bytes on the protocol CDC stream. UART
stdio is also disabled.

The reference USB carrier defaults to the development identity `CAFE:CEC1`.
`CEC_INJECTOR_USB_VID`, `CEC_INJECTOR_USB_PID`, and the manufacturer/product
strings are build-time settings. These values identify a reference build, not
Device Protocol v1, and must be replaced with legitimately assigned values for
distribution where required.

The single CDC ACM function has two interfaces and no HID function. Its serial
string is generated from the RP2040 unique board identifier; no fixed board
serial is stored in the repository. USB suspend preserves the transport
attachment session. The production composition connects TinyUSB lifecycle
callbacks and Remote Wake delivery through the platform binding.

Use the canonical [build guide](../../docs/building.md) for validated dependency
versions, host tests, Debug and Release commands, configuration settings, and
artifact locations. The [hardware guide](../../hardware/README.md) documents
only the wiring and limitations of the validated Experimental prototype. The
[validation and compatibility record](../../docs/validation.md) identifies the
exact tested firmware, environment, and evidence boundaries.
