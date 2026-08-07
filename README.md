# CEC Injector

CEC Injector is a programmable HDMI-CEC endpoint for open computing platforms.
It aims to provide the everyday HDMI-CEC experience expected from consumer
devices while preserving deeper programmability, extensibility, and user
control.

A Device participates directly on the CEC bus and exposes CEC capabilities to
an attached host. It can also perform bounded delivery functions that must work
while host software cannot, such as requesting an authorized host wake after a
precisely configured CEC trigger. It does not decide user-facing power, input,
or application policy.

> **Device owns delivery. Mate owns host policy.**

CEC Injector Mate is the project's host-software role. Mate is intentionally
outside this Device repository, and independent host implementations can use
the public protocol without it.

## Current status

This repository contains:

- the published, **Experimental** [CEC Injector Device Protocol
  v1](protocol/README.md), including the **Experimental** Wake Extension v1;
- a complete platform-neutral [DeviceCore](device-core/README.md);
- the production [RP2040 reference firmware](platforms/rp2040/README.md); and
- deterministic [host, conformance, and platform tests](tests/README.md).

RP2040 is currently the only supported reference implementation. It is not the
definition of CEC Injector: future Device implementations may use other MCUs,
transports, or internal strategies while conforming to the applicable public
contracts.

The current RP2040 firmware has been validated on real hardware, including its
normal host lifecycle and Remote Wake path. See the [build
guide](docs/building.md); the versioned validation record is scheduled as the
next publication work.

The electrical prototype remains **Experimental**. It is not presented as a
production-ready, certified, or universally safe HDMI design. See the current
[hardware status](hardware/README.md).

## What it can do today

The RP2040 reference firmware implements:

- the published Pulse-Eight-compatible serial profile subset;
- bidirectional physical CEC receive and transmit;
- follower-ACK behavior for configured logical addresses;
- Wake Extension v1 configuration, matching, lifecycle, and diagnostics;
- USB Remote Wake delivery; and
- transport attachment, suspend, resume, and session-reset handling.

These are Device capabilities and delivery mechanisms. Decisions such as when
the host should suspend, which input a user prefers, or what to do after resume
remain host policy.

## What is not claimed

The project does not claim:

- HDMI certification or production-ready electronics;
- universal TV, vendor, or HDMI-system compatibility;
- full Pulse-Eight protocol compatibility;
- compatibility with the kernel `pulse8-cec` driver;
- that future Devices must use RP2040 or USB; or
- Stable protocol maturity.

## Documentation

- [Protocol documentation](protocol/README.md) — normative Device/host
  contracts and conformance claims.
- [Design Charter](docs/design-charter.md) and [Russian
  translation](docs/design-charter.ru.md) — project goals and architectural
  boundaries.
- [DeviceCore](device-core/README.md) — platform-neutral implementation scope.
- [Build guide](docs/building.md) — host tests and reproducible RP2040 Debug and
  Release builds.
- [RP2040 reference platform](platforms/rp2040/README.md) — current platform,
  build entry point, and USB/electrical boundaries.
- [Hardware](hardware/README.md) — experimental hardware status.
- [Tests](tests/README.md) — automated test scopes and host test commands.
- [Third-party material](THIRD_PARTY.md) — provenance and retained licenses.

## Repository areas

- `protocol/` — the public Device/host contract.
- `device-core/` — platform-neutral Device behavior and delivery semantics.
- `platforms/` — platform-specific Device implementations.
- `hardware/` — electrical, PCB, and enclosure material when available.
- `tools/` — public Device development and validation tools when available.
- `tests/` — host, protocol-conformance, and platform tests.

## License

Project-owned repository content is licensed under the [Apache License
2.0](LICENSE). Third-party-derived material retains its applicable attribution
and license; see [THIRD_PARTY.md](THIRD_PARTY.md). Future hardware design
sources may use a separate hardware-specific license when introduced.
