# Protocol

This area contains the public protocol between Device and host implementations.
The protocol is the long-term engineering contract: host intent crosses this
boundary, while capability ownership and delivery remain with the Device.

The normative specification is [CEC Injector Device Protocol
v1](specification.md). Its current maturity is **Experimental**: v1 is intended
to be implementable and its published meaning is fixed, but future incompatible
protocol versions remain possible.

Normative documents:

- [CEC Injector Device Protocol v1](specification.md) — architecture,
  terminology, lifecycle, and specification structure.
- [Pulse-Eight Serial Transport Binding](pulse8-serial-binding.md) — framing
  over an ordered octet stream.
- [Pulse-Eight Compatibility Profile](pulse8-compatibility-profile.md) — the
  supported adapter command subset, runtime state, CEC TX/RX, and follower ACK
  behavior.
- [Wake Extension v1](wake-extension-v1.md) — the CEC Injector project command
  namespace and the transport-assisted host wake capability.
- [Device Protocol v1 Conformance](conformance.md) — composable Device and host
  conformance claims and observable test boundaries.
- [Device Protocol v1 Numeric Registry](numeric-registry.md) — an index of
  frozen values linked to their authoritative definitions.

Navigation:

```text
specification
|-- pulse8-serial-binding
|   `-- pulse8-compatibility-profile
|       `-- wake-extension-v1
|-- conformance
`-- numeric-registry
```

The tree shows the initial published document set and its navigation path. It
does not require every future extension to depend on the Pulse-Eight profile.
