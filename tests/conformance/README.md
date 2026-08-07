# Protocol conformance tests

These tests exercise externally observable requirements from the normative
protocol documents through public DeviceCore APIs. They use deterministic
vectors derived from the specifications rather than private captures or
implementation-state inspection.

The current suites cover:

- the [Pulse-Eight Serial Transport
  Binding](../../protocol/pulse8-serial-binding.md), including malformed input
  and resynchronization;
- the [Pulse-Eight Compatibility
  Profile](../../protocol/pulse8-compatibility-profile.md), including command
  handling and CEC transmit transactions;
- CEC receive representation and follower-ACK decisions defined by that
  profile; and
- [Wake Extension v1](../../protocol/wake-extension-v1.md), including commands,
  matching, lifecycle, diagnostics, and remote-wake outcomes.

CTest provides automated conformance evidence for these observable software
boundaries. It does not establish physical electrical behavior or compatibility
with untested hardware.
