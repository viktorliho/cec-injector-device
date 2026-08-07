# Wake Extension v1

**Part of CEC Injector Device Protocol v1**

**Maturity: Experimental**

## Status

This document normatively defines Wake Extension version 1 for implementations
that claim support for it.

Wake Extension v1 is a Device delivery capability. It lets a host discover,
configure, arm, inspect, and disarm one precisely defined CEC-triggered remote
wake mechanism. It does not define host policy or authorize unrelated CEC
actions.

## 1. Scope and Architectural Boundary

The extension allows a host to:

- discover Wake Extension capabilities;
- configure an exact CEC wake target;
- arm and disarm wake delivery;
- inspect configuration and current lifecycle state;
- inspect diagnostic counters and the last wake reason; and
- clear last-wake diagnostic information.

When the capability is configured, enabled, armed, suspended, and authorized,
the Device may recognize the trigger defined in Section 14 and invoke a
binding-provided remote-wake mechanism.

The Device owns that bounded delivery behavior. It MUST NOT use Wake Extension
state to invent user-facing host intent, transmit a CEC message, select an HDMI
input, power a television, declare the host an active source, or decide what
the host does after resume. Decisions about normal suspension, display power,
preferred input, and post-resume policy are outside this extension.

## 2. Normative Dependencies

Wake Extension messages are decoded messages carried by the
[Pulse-Eight Serial Transport Binding](pulse8-serial-binding.md). All layouts
in this document are expressed before serial escaping.

The generic `COMMAND_ACCEPTED` (`0x08`) and `COMMAND_REJECTED` (`0x09`)
responses are defined by the
[Pulse-Eight Compatibility Profile](pulse8-compatibility-profile.md). This
extension uses those responses without adding an error-reason field.

Multi-octet integers in this document are unsigned and encoded
most-significant octet first.

## 3. Binding-Provided Remote Wake

Wake Extension v1 does not require a particular transport or implementation.
A binding or carrier claiming support for this extension MUST provide:

- a suspended state distinct from detach;
- an indication of whether remote wake is authorized when the binding requires
  authorization;
- an operation by which the Device can request host wake or resume; and
- success or failure reporting for that operation.

That binding MUST define how its lifecycle and authorization events map to the
terms **transport configured**, **suspended**, **remote wake authorized**, and
**remote-wake request** used here. A binding that cannot provide these
semantics MUST NOT claim Wake Extension v1 support.

The result of the binding-provided operation reports whether the delivery
request was accepted or failed. It does not prove that the host completed
resume.

## 4. CEC Injector Project Command Namespace

Device Protocol v1 allocates decoded-message command identifiers `0xE0`
through `0xE7` to CEC Injector project extensions. Every identifier in that
range is assigned to Wake Extension v1:

| ID | Command |
|---:|---|
| `E0` | `GET_WAKE_CAPABILITIES` |
| `E1` | `SET_WAKE_CONFIG` |
| `E2` | `GET_WAKE_CONFIG` |
| `E3` | `ARM_WAKE` |
| `E4` | `DISARM_WAKE` |
| `E5` | `GET_WAKE_STATUS` |
| `E6` | `GET_LAST_WAKE_REASON` |
| `E7` | `CLEAR_LAST_WAKE_REASON` |

These are CEC Injector project commands, not upstream Pulse-Eight commands.
Device Protocol v1 allocates no identifier at or above `0xE8`. This absence of
allocation does not declare the remaining range permanently available.

## 5. Versioning

The Wake Extension wire version is `0x01`. It is independent of Device Protocol
v1, implementation release versions, and Pulse-Eight compatibility metadata.

Every Device response containing an extension-version field MUST emit `0x01`.
A host request supplying an extension version MUST supply `0x01`; another value
MUST be rejected as specified for that command. Wake Extension versioning does
not add a generic Device Protocol version field.

## 6. Command Registry

The payload column excludes the command identifier. Request and response
lengths are exact.

| ID | Command | Request payload | Success response | State mutation | Possible external action | Prerequisite |
|---:|---|---|---|---|---|---|
| `E0` | `GET_WAKE_CAPABILITIES` | none | `E0 01 01 00` | none | none | none |
| `E1` | `SET_WAKE_CONFIG` | `01 <enabled> <pa_hi> <pa_lo> 01` | `08 E1` | replaces config and disarms | none | valid fields |
| `E2` | `GET_WAKE_CONFIG` | none | `E2 01 <valid> <enabled> <pa_hi> <pa_lo> <mode>` | none | none | none |
| `E3` | `ARM_WAKE` | none | `08 E3` | arms and clears pending trigger | none | valid enabled config; not suspended |
| `E4` | `DISARM_WAKE` | none | `08 E4` | disarms and clears pending trigger | none | none |
| `E5` | `GET_WAKE_STATUS` | none | 32-octet status | none | none | none |
| `E6` | `GET_LAST_WAKE_REASON` | none | `8 + frame_length` octets | none | none | none |
| `E7` | `CLEAR_LAST_WAKE_REASON` | none | `08 E7` | clears last-reason diagnostics | none | none |

A request with the wrong length MUST produce `COMMAND_REJECTED` for the
original command and MUST NOT perform that command's mutation or action.

## 7. Capabilities — GET_WAKE_CAPABILITIES (`0xE0`)

The exact request is:

```text
E0
```

The exact Wake Extension v1 response is:

```text
E0 01 01 00
```

| Offset | Field | Value and meaning |
|---:|---|---|
| 0 | command | `E0` |
| 1 | version | `01`, Wake Extension version 1 |
| 2 | modes | `01`, mode-support bitmap |
| 3 | flags | `00`, capability-flags bitmap |

For the modes bitmap, bit `N - 1` represents positive mode value `N`.
Therefore bit 0 advertises mode 1. The one-octet bitmap can directly represent
positive mode values 1 through 8. Wake Extension v1 defines only mode 1.

The Device MUST emit unsupported and reserved bits in both bitmaps as zero. A
host SHOULD ignore unknown set bits in the `modes` and `flags` fields because
both are explicitly forward-compatible bitmaps. That rule does not apply to
mode values or other enum fields.

## 8. Wake Configuration

Wake configuration consists of:

- a validity flag;
- an enabled Boolean;
- an unsigned 16-bit CEC physical address; and
- a one-octet wake mode.

This state is independent of the Pulse-Eight profile's runtime physical
address. Updating one MUST NOT implicitly update the other.

### 8.1 Physical-Address Validation

A Wake physical address is invalid if it is `0x0000` or `0xFFFF`.

For every other address, its four nibbles are examined from most significant to
least significant. After the first zero nibble, every following nibble MUST
also be zero. For example, `1.0.0.0` and `1.2.0.0` are valid, while `1.0.2.0`
is invalid.

### 8.2 SET_WAKE_CONFIG (`0xE1`)

The exact request is:

```text
E1 01 <enabled> <pa_hi> <pa_lo> 01
```

The request is valid only when:

- its decoded length is exactly six octets;
- version is `0x01`;
- `enabled` is exactly `0x00` or `0x01`;
- the physical address passes Section 8.1; and
- mode is `0x01`, `SET_STREAM_PATH`.

A valid request atomically replaces the complete Wake configuration, marks it
valid, stores all supplied fields, disarms Wake Extension, and clears any
pending or already-consumed wake request for the current lifecycle state. It
produces:

```text
08 E1
```

Replacement preserves diagnostic counters and last-wake diagnostic fields.
An enabled value of `0x00` is a valid configuration, but it cannot be armed and
has derived state 0.

An invalid request produces:

```text
09 E1
```

Invalid E1 is non-destructive. It MUST preserve the previous configuration
byte-for-byte, preserve armed state, preserve pending or consumed wake state,
preserve diagnostics, and cause no remote-wake or CEC action.

### 8.3 GET_WAKE_CONFIG (`0xE2`)

The exact request is `E2`. The exact response layout is seven decoded octets:

```text
E2 01 <valid> <enabled> <pa_hi> <pa_lo> <mode>
```

`valid` and `enabled` are encoded as `0x00` or `0x01`. After reset, the exact
response is:

```text
E2 01 00 00 FF FF 00
```

## 9. Arming and Disarming

### 9.1 ARM_WAKE (`0xE3`)

The exact request is `E3`. It succeeds only when configuration is valid,
configuration is enabled, and the binding is not suspended.

Success sets armed to true, clears a pending or consumed wake request, preserves
configuration and diagnostics, and produces:

```text
08 E3
```

The operation is idempotent when already armed and otherwise legal. Remote-wake
authorization is not an ARM prerequisite; it is required for wake eligibility
during suspend. ARM acceptance causes no wake or CEC action.

If a prerequisite is absent, the Device produces `09 E3` and preserves state.

### 9.2 DISARM_WAKE (`0xE4`)

The exact request is `E4`. It is accepted regardless of configuration validity,
enabled state, or previous armed state.

Acceptance sets armed to false, clears a pending or consumed wake request,
preserves configuration and diagnostics, and produces:

```text
08 E4
```

DISARM causes no remote-wake or CEC action.

## 10. Wake Status — GET_WAKE_STATUS (`0xE5`)

The exact request is `E5`. The response contains exactly 32 decoded octets:

| Offset | Width | Field | Encoding |
|---:|---:|---|---|
| 0 | 1 | command | `E5` |
| 1 | 1 | version | `01` |
| 2 | 1 | state | derived state from Section 12 |
| 3 | 1 | flags | bitmap from Section 11 |
| 4 | 2 | configured physical address | `u16be` |
| 6 | 1 | mode | enum |
| 7 | 4 | `suspend_cycles` | `u32be` |
| 11 | 4 | `matching_frames` | `u32be` |
| 15 | 4 | `rejected_frames` | `u32be` |
| 19 | 4 | `remote_wakeup_requests` | `u32be` |
| 23 | 4 | `duplicate_suppressed` | `u32be` |
| 27 | 1 | `last_result` | enum from Section 13 |
| 28 | 4 | `last_wake_offset_us` | `u32be` diagnostic value |

No field may be added to or omitted from this v1 layout.

## 11. Status Flags

The E5 flags octet is:

| Bit | Value | Meaning |
|---:|---:|---|
| 0 | `0x01` | Wake configuration is valid |
| 1 | `0x02` | Wake configuration is enabled |
| 2 | `0x04` | Wake Extension is armed |
| 3 | `0x08` | the binding reports the transport configured |
| 4 | `0x10` | the binding reports suspended |
| 5 | `0x20` | the binding reports host remote wake authorized |
| 6 | `0x40` | a wake request was issued or consumed in this suspend cycle |
| 7 | `0x80` | reserved; Device MUST emit zero |

Bits 0 through 6 expose current configuration and lifecycle facts and are
authoritative for those facts. They do not themselves authorize host policy.
Bit 7 is reserved for compatible expansion; a host SHOULD ignore it if set by
a future extension version.

## 12. Derived State Values

E5 state is a compact summary derived in the following precedence order:

| Value | Condition |
|---:|---|
| `0` | no valid configuration, or configuration is disabled |
| `4` | valid enabled configuration and a wake request is consumed in the current suspend cycle |
| `3` | valid enabled configuration, armed, suspended, and remote wake authorized |
| `2` | valid enabled configuration and armed, but not in the authorized suspended condition for state 3 |
| `1` | valid enabled configuration and not armed |

The individual E5 flags are the explicit representation of the contributing
conditions. Hosts SHOULD use them when the distinction matters. No other state
value is defined by Wake Extension v1.

## 13. Wake Result Values

The result enum used by E5 and E6 is:

| Value | Name | Meaning |
|---:|---|---|
| `0` | `NONE` | no retained remote-wake delivery result |
| `1` | `SUCCESS` | the binding-provided remote-wake operation reported success |
| `2` | `REMOTE_WAKE_FAILED` | the operation was attempted and reported failure |

`SUCCESS` does not prove that host resume completed. `REMOTE_WAKE_FAILED` does
not identify why the binding failed. No other result value is defined in Wake
Extension v1.

## 14. Exact Wake Trigger and Eligibility

Mode 1 is `SET_STREAM_PATH`. Its only matching CEC frame is exactly four
octets:

```text
0F 86 <pa_hi> <pa_lo>
```

The header means initiator logical address `0` (TV) and destination `0xF`
(broadcast). Opcode `0x86` is CEC `Set Stream Path`. The two operands MUST equal
the configured Wake physical address byte-for-byte in big-endian order.

A frame matches only if it also:

- has exactly four CEC octets;
- has a valid final EOM indication;
- has no receive error;
- did not overlap a local CEC transmission; and
- was not affected by receive-analysis overflow.

A matching frame is wake-eligible only when configuration is valid and enabled,
the Device is armed, the binding is suspended, remote wake is authorized, and
no wake request or matching trigger is already pending for the suspend cycle.
The frame's observed start MUST be at or after the start of that suspend
interval.

No other initiator, destination, opcode, operand length, physical address, or
malformed/local-overlap frame is a Wake Extension v1 trigger. The Device MUST
NOT broaden matching based on television vendor behavior.

## 15. Wake Delivery and Duplicate Suppression

When an eligible frame is accepted, the Device records its four bytes and its
offset from the start of the suspend interval, increments `matching_frames`,
and marks one delivery pending.

The Device MUST NOT invoke the binding-provided remote-wake operation before
10,000 microseconds have elapsed from the beginning of the suspend interval. A
matching frame received earlier remains pending until that earliest request
time, provided eligibility still holds. A binding MAY impose a later permitted
time; the Device MUST obey the binding.

Immediately before delivery, the Device MUST recheck that configuration remains
valid and enabled, armed state remains true, the binding remains suspended and
authorized, and no request has already been consumed. If the recheck fails,
the pending match is rejected without invoking remote wake.

On delivery, the Device marks the current suspend cycle consumed, increments
`remote_wakeup_requests`, invokes the binding-provided operation exactly once,
and records `SUCCESS` or `REMOTE_WAKE_FAILED`. It MUST NOT automatically retry a
failed or successful request during that suspend cycle.

While a matching trigger is pending or the cycle is consumed, another exact
matching frame increments `duplicate_suppressed` and causes no further request.
Nonmatching frames in that condition cause neither a request nor a duplicate
increment.

## 16. Diagnostic Counters

All E5 counters are diagnostic, unsigned 32-bit integers. They wrap modulo
2^32. Control behavior MUST NOT depend on their exact values.

| Counter | Increment event |
|---|---|
| `suspend_cycles` | each binding-reported transition into a suspend interval, regardless of armed or authorization state |
| `matching_frames` | an exact matching frame is accepted as eligible and becomes pending |
| `rejected_frames` | a frame is rejected under the categories below |
| `remote_wakeup_requests` | the binding-provided remote-wake operation is invoked |
| `duplicate_suppressed` | another exact match arrives while a match is pending or the suspend cycle is consumed |

`rejected_frames` increments for these frozen categories:

- while suspended, a non-null frame whose observed start precedes the current
  suspend interval;
- a nonmatching frame offered while suspended, unless duplicate handling has
  already consumed the offer;
- an exact match that is not wake-eligible when offered; or
- a pending eligible match that fails the delivery-time recheck.

A nonmatching frame offered while awake does not increment `rejected_frames`.
While duplicate handling is active, a nonmatching frame does not increment it.
These counters classify observations; they are not error codes or proof of a
specific historical event.

Counters survive parser recovery, suspend, resume, configuration replacement,
arm, disarm, and E7. They reset to zero when the transport attachment session
ends, on Device reset, or on power loss.

## 17. Last-Wake Offset

`last_wake_offset_us` is the observed start time of the most recently accepted
eligible matching frame minus the start time of its suspend interval, expressed
as unsigned microseconds.

If the difference exceeds `UINT32_MAX`, the field saturates at `0xFFFFFFFF`; it
does not wrap. The value is diagnostic and makes no guarantee about clock
precision or host wake latency.

The field is updated when an eligible match becomes pending, before the remote
wake operation is attempted. It survives suspend/resume, configuration changes,
arm, and disarm. E7 or an attachment-session/reset boundary clears it to zero.

## 18. Last Wake Reason — GET_LAST_WAKE_REASON (`0xE6`)

The exact request is `E6`. The response contains `8 + frame_length` decoded
octets:

| Offset | Width | Field | Encoding |
|---:|---:|---|---|
| 0 | 1 | command | `E6` |
| 1 | 1 | version | `01` |
| 2 | 1 | result | enum from Section 13 |
| 3 | 4 | `last_wake_offset_us` | `u32be` |
| 7 | 1 | `frame_length` | number of following frame octets |
| 8 | `frame_length` | frame | captured CEC octets in bus order |

`frame_length` is in the inclusive range 0 through 16. Under Wake Extension v1
trigger rules, a retained triggering frame has length 4; zero means no retained
frame. A reset or E7 therefore produces exactly:

```text
E6 01 00 00 00 00 00 00
```

E6 is diagnostic. Its result, offset, and frame fields are independently
updated by the frozen lifecycle: frame and offset are recorded when a match is
accepted, while result is recorded when remote-wake delivery is attempted.
Hosts MUST NOT assume these fields form an atomic historical-event record.

### 18.1 CLEAR_LAST_WAKE_REASON (`0xE7`)

The exact request is `E7`. Acceptance clears result to `NONE`, offset to zero,
frame length to zero, and all retained frame bytes, then produces:

```text
08 E7
```

E7 preserves Wake configuration, armed state, pending or consumed state,
lifecycle flags, and all counters. It causes no wake or CEC action.

## 19. Lifecycle and State Lifetime

### 19.1 Initial State and Attachment

After Device initialization, attachment-session end, Device reset, or power
loss, Wake state has these safe defaults:

- configuration invalid and disabled;
- configured physical address `0xFFFF`;
- mode `0x00`;
- armed false;
- no pending or consumed wake request;
- transport lifecycle flags false until reported by the binding;
- all counters zero;
- result `NONE`, offset zero, and no retained frame; and
- derived state 0.

Attachment or transport configuration marks the binding configured and clears
suspended, authorization, pending, and consumed state. It does not itself
create wake intent or invoke remote wake.

### 19.2 Suspend

Entering suspend preserves configuration, armed state, counters, and
last-reason diagnostics. It sets suspended, records current remote-wake
authorization, clears pending and consumed state, increments
`suspend_cycles`, and starts a new suspend interval.

An armed configuration remains armed. Its derived state becomes 3 only when
authorization is present; without authorization it remains state 2.

### 19.3 Resume

Resume preserves configuration, armed state, counters, and last-reason
diagnostics. It clears suspended, authorization, pending, and consumed state.
An enabled armed configuration therefore returns to derived state 2.

### 19.4 Parser Recovery

Parser recovery alone changes no Wake configuration, armed state, lifecycle
state, pending state, counter, or diagnostic field. It does not begin a new
transport attachment session.

### 19.5 Detach, Binding Reset, Device Reset, and Power Loss

An event ending the transport attachment session clears all volatile Wake
Extension state to Section 19.1 defaults. No pending wake request or action may
be replayed after a later attachment.

Wake Extension v1 defines no persistent configuration.

## 20. Failure and Safety Requirements

Malformed framing is discarded silently by the serial binding. A validly
framed Wake command with an invalid length or value is rejected through the
generic profile response.

Invalid E1 MUST be non-destructive. Unsupported version or mode values MUST NOT
be interpreted as the closest supported value. Unknown enum values are not
reserved bitmap bits.

No command in this extension directly transmits CEC. ARM, attachment, resume,
configuration, and status inspection MUST NOT themselves invoke remote wake.
Only the exact eligible trigger may create a pending delivery request.

Detach, reset, stale frames from before the suspend interval, duplicate trigger
frames, and failed delivery MUST NOT cause action replay or automatic retry.

## 21. Conformance

An implementation claiming Wake Extension v1 MUST:

- implement all commands `0xE0` through `0xE7` with the exact layouts and
  mutations in this document;
- implement mode 1 with the exact matcher and eligibility rules;
- provide the binding lifecycle and remote-wake mapping required by Section 3;
- preserve and reset state according to Section 19;
- emit reserved outgoing bits as zero;
- implement one request at most per suspend cycle and no automatic retry; and
- expose the specified diagnostics without treating counters as control state.

An implementation MUST NOT claim Wake Extension v1 while implementing only a
subset of E0–E7 or while assigning different meanings to their fields.

## 22. Validation Status (Non-Normative)

The reference implementation realizes the binding-provided delivery mechanism
with USB remote wake on an RP2040 and has been hardware-validated with a Philips
55PUS9010/12. These facts describe validation scope only. USB, RP2040, and that
television model are not normative requirements and do not broaden the trigger
contract to vendor-specific behavior.
