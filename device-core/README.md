# DeviceCore

DeviceCore implements the platform-neutral Device behavior used by the
reference firmware. Its scope is capability and delivery logic; user-facing
host policy belongs to Mate.

The current modules provide:

- volatile adapter and Pulse-Eight profile state;
- the Pulse-Eight Serial Transport Binding parser and encoder;
- Pulse-Eight Compatibility Profile command dispatch and responses;
- CEC transmit transaction assembly and terminal-result handling;
- CEC receive decoding and asynchronous profile representation;
- follower-ACK eligibility decisions; and
- Wake Extension v1 codec, matcher, lifecycle, delivery state, and diagnostics.

The CMake target `CECInjector::DeviceCore` is a host-buildable C library. It has
no dependency on RP2040, the Pico SDK, TinyUSB, GPIO, or a particular physical
wake backend. Platform code supplies electrical observations, physical CEC
transmit and ACK operations, transport lifecycle, and remote-wake delivery.

The implementation API is Experimental project-internal integration surface;
it does not promise a stable external C ABI. Observable compatibility is
defined by the [normative protocol documents](../protocol/README.md), not by
the source module layout.
