# Tests

The public automated suite covers platform-neutral Device behavior, observable
protocol conformance, and RP2040 integration boundaries.

Current scopes are:

- adapter-state defaults, mutation, and reset;
- Pulse-Eight serial framing, escaping, limits, and recovery;
- Pulse-Eight Compatibility Profile commands and CEC TX transactions;
- CEC receive decoding and follower-ACK decisions;
- Wake Extension v1 commands, matching, lifecycle, and diagnostics;
- RP2040 USB transport, output, and attachment lifecycle;
- RP2040 USB Remote Wake binding; and
- production-composition RX, TX, Wake, and session flows through platform
  stubs.

Run the complete host suite without the Pico SDK or an Arm toolchain:

```sh
cmake --preset host-debug
cmake --build --preset host-debug
ctest --preset host-debug
```

The deterministic host suite does not prove electrical timing, signal quality,
USB behavior on physical hardware, or compatibility with a particular TV or
host environment. Those require separate hardware validation.
