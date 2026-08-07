# Protocol conformance tests

This area will contain black-box tests of externally observable Device Protocol
v1 behavior. Tests must exercise protocol input, output, and lifecycle stimuli;
they must not define compatibility by inspecting private implementation state.

The serial-binding test exercises framing through the public DeviceCore API.
Future conformance vectors will also cover:

- the [Pulse-Eight Serial Transport
  Binding](../../protocol/pulse8-serial-binding.md);
- the [Pulse-Eight Compatibility
  Profile](../../protocol/pulse8-compatibility-profile.md); and
- [Wake Extension v1](../../protocol/wake-extension-v1.md).

The tests use deterministic vectors derived from the normative documents; raw
private captures are not inputs.
