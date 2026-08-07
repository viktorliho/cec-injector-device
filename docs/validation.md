# Validation and Compatibility Record

**NON-NORMATIVE**

## Status and purpose

This document records validation evidence for one CEC Injector Device
implementation in one tested hardware and host environment. It is not a
protocol specification, compatibility certification, HDMI certification, or
electrical-safety certification.

Normative behavior and conformance claims remain defined by the
[`protocol/`](../protocol/README.md) documents. Successful validation here does
not turn the tested MCU, transport, host software, operating system, or TV into
a protocol requirement.

## Validated implementation

The hardware-validated firmware is identified by both its source and exact
binary:

| Item | Identity |
|---|---|
| Public source commit | `35d9df8f31d5f1973292ae72a1e548d87a6b632a` |
| Release UF2 SHA-256 | `ca9e2f546077ac209fe6be399c0eab085db06115611109215cdc88c22b475a05` |

The commit identifies the firmware source. The SHA-256 value identifies the
exact Release UF2 installed for hardware validation. Later documentation-only
commits do not change that source identity, and later firmware changes do not
inherit this hardware-validation status without new evidence.

## Automated validation

At the validated source commit:

- the complete host test run passed: 8/8 tests;
- host tests covered adapter state and platform-neutral DeviceCore behavior;
- conformance suites exercised serial framing, the Pulse-Eight Compatibility
  Profile, CEC RX/follower-ACK decisions, and Wake Extension v1;
- RP2040 platform tests exercised USB transport, lifecycle, Remote Wake
  binding, and production-composition boundaries; and
- clean RP2040 Debug and Release cross-builds produced the production
  `cec_adapter` target.

The validated cross-build environment was:

| Dependency | Validated version |
|---|---|
| Raspberry Pi Pico SDK | tag `2.2.0`, commit `a1438dff1d38bd9c65dbd693f0e5db4b9ae91779` |
| TinyUSB from the Pico SDK checkout | commit `86ad6e56c1700e85f1c5678607a762cfe3aa2f47` |
| Arm GNU Toolchain | `14.2.Rel1`; GCC `14.2.1 20241119` |

See the [build guide](building.md) for the reproducible workflows. Automated
tests and cross-builds do not prove electrical timing or behavior on physical
HDMI and USB hardware.

## Validation environment

### Hardware

- Device: RP2040 reference implementation from this repository;
- electrical connection: the [documented Experimental
  prototype](../hardware/README.md); and
- TV: Philips `55PUS9010/12`.

This single model-specific result is not a claim about other Philips products.

### Host

- SteamOS host in the existing production setup;
- existing project daemon/Mate stack, unchanged during drop-in validation; and
- existing explicit-port libCEC integration, unchanged during validation.

The public UF2 replaced the already-working private firmware without changes to
daemon behavior or configuration, libCEC integration, udev/systemd setup, USB
identity matching, permissions, or SteamOS policy. This establishes drop-in
firmware compatibility for that tested host setup. SteamOS and that host stack
are validation scope, not Device Protocol requirements.

## Protocol and features exercised

Hardware validation exercised the reference implementation's initial public
composition:

- CEC Injector Device Protocol v1;
- Pulse-Eight Serial Transport Binding;
- Pulse-Eight Compatibility Profile; and
- Wake Extension v1.

This records behavior of the documented project profile and extension in the
tested environment. It does not claim full upstream Pulse-Eight compatibility.

## Physical behavior validated

The following behavior was observed successfully:

- USB enumeration and the expected reference identity/interface layout;
- stable per-board serial device path across firmware replacement and host
  lifecycle transitions;
- daemon selection and libCEC handshake without host-side changes;
- physical bidirectional CEC receive and transmit;
- controlled operation and follower-ACK behavior;
- remote-control navigation without observed missing or duplicate actions;
- TV-originated standby causing one host suspend request;
- controller resume restoring the TV, HDMI input, daemon, libCEC, and remote
  input;
- CEC-triggered USB Remote Wake;
- physical USB disconnect/reconnect and attachment-session recovery; and
- normal host reboot with automatic restoration of the existing integration.

Follower-ACK evidence is indirect: successful controlled operation, complete
direct frames, stable libCEC communication, and reliable remote traffic were
observed. Electrical ACK waveform timing was not characterized with an
oscilloscope as part of this validation record.

## Wake validation

Five physical CEC-triggered Remote Wake cycles completed successfully. Each
retained wake reason reported:

```text
result: SUCCESS
frame:  0F:86:10:00
```

No duplicate wake was observed.

Negative cases exercised:

- selecting a different HDMI path did not wake the host; and
- a matching trigger did not wake the host while Wake was disarmed.

The host-authorization-disabled USB Remote Wake negative was not performed.

## Lifecycle and repeated-cycle confidence

The validation run included:

- at least two controller suspend/resume cycles;
- five successful CEC-triggered Remote Wake cycles;
- one physical Injector USB disconnect/reconnect; and
- one normal SteamOS reboot.

Remote navigation was checked after major transitions. No stale output or
replayed action, duplicate input, wedged CDC transport, lost Wake arm, or
recovery failure was observed. This was a modest confidence run, not
long-duration endurance or stress testing.

## Known environment-specific observation

With the Philips `55PUS9010/12`, an Active Source operation may be reported as
timed out by the host path even though the TV visibly performs the requested
power/input action. This is an observed vendor/environment behavior, not Device
Protocol v1 behavior.

## Compatibility record

This document is the authoritative repository record of hardware compatibility
evidence until additional reviewed validation records are published.

| TV/model | Device implementation | Host environment | Validation level | Notes |
|---|---|---|---|---|
| Philips `55PUS9010/12` | RP2040 reference firmware at the source commit and UF2 hash above | SteamOS production setup with existing daemon/Mate and explicit-port libCEC integration | Hardware validated for the scenarios in this document | Active Source may be reported as timed out despite visible success |

## Not validated

The current evidence does not establish:

- compatibility with other TV models or vendors;
- compatibility with other RP2040 boards unless separately tested;
- another MCU or platform implementation;
- another electrical frontend;
- HDMI certification or production electrical safety;
- commercial-production hardware readiness;
- kernel `pulse8-cec` compatibility;
- automatic libCEC discovery;
- every possible upstream Pulse-Eight or libCEC mode;
- the USB wake-authorization-disabled negative case; or
- long-duration endurance or stress behavior.
