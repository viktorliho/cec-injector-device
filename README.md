# CEC Injector

CEC Injector is a programmable HDMI-CEC endpoint. The project exists to bring
consumer-grade HDMI-CEC integration to open computing platforms without giving
up flexibility and control.

This repository contains the Device side of the CEC Injector ecosystem. The
Device owns capabilities and delivery of already-defined intent. User-facing
host policy belongs to CEC Injector Mate and is intentionally kept outside this
repository.

The public protocol is the long-term engineering contract between those roles,
allowing independent Device and host implementations. Its maturity is
**Experimental**: the published meaning of Device Protocol v1 is fixed, while
future incompatible protocol versions remain possible. See the
[protocol documentation](protocol/README.md).

RP2040 is presently the only supported platform implementation. It is one
implementation of the Device architecture, not the definition of CEC Injector;
future implementations may use different MCUs, transports, or internal
strategies.

The current electrical prototype is experimental. It is not presented as a
production-ready, certified, or universally safe HDMI design.

See the [Design Charter](docs/design-charter.md) for the project's architectural
principles and boundaries.

## Repository areas

- `protocol/` — the public Device/host contract.
- `device-core/` — platform-independent Device behavior and delivery semantics.
- `platforms/` — platform-specific Device implementations, currently RP2040.
- `hardware/` — electrical, PCB, and enclosure design material when published.
- `tools/` — development tools scoped to the Device repository.
- `tests/` — Device tests, protocol conformance tests, and public fixtures.

No implementation has been migrated at Repository Genesis. License selection is
also intentionally deferred for future hardware design sources.

## License

The repository content currently published here is licensed under the
[Apache License 2.0](LICENSE). Future PCB, schematic, enclosure, and other
hardware design sources may use a separate hardware-specific license when they
are introduced.
