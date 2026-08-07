# Platforms

Platform implementations connect DeviceCore to concrete electrical, timing,
transport, lifecycle, and wake-delivery mechanisms.

[`rp2040/`](rp2040/README.md) is currently the only supported reference
implementation. RP2040 is a current implementation choice, not an architectural
requirement. Future platforms may use different MCUs, transports, or internal
strategies while conforming to the protocol parts they claim.
