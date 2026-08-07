# Pulse-Eight Compatibility Profile

**Part of CEC Injector Device Protocol v1**

**Maturity: Experimental**

## Status

This document defines a normative compatibility profile for implementations
that claim support for it. It provides partial compatibility with the
documented subset of the Pulse-Eight adapter protocol defined below.

This profile does not claim full Pulse-Eight compatibility, Pulse-Eight USB
identity, automatic adapter discovery, Linux kernel `pulse8-cec`
compatibility, persistent adapter configuration, every upstream command or
adapter mode, or autonomous host or application policy.

## 1. Scope

This profile defines the meaning of decoded messages carried by the
[Pulse-Eight Serial Transport Binding](pulse8-serial-binding.md). It defines:

- the request, response, and asynchronous-message model;
- the supported Pulse-Eight-compatible command subset;
- volatile adapter runtime state;
- CEC transmit assembly and terminal results;
- asynchronous CEC receive delivery;
- controlled follower acknowledgement behavior; and
- rejection and lifecycle behavior within this profile.

This profile does not define the CEC Injector project extension namespace or
Wake Extension v1. It also does not define a carrier, USB identity, electrical
CEC implementation, host policy, or implementation architecture.

## 2. Relationship to the Serial Binding

Every message in this profile is one decoded frame produced by the Pulse-Eight
Serial Transport Binding. The binding's framing, escaping, decoded-size limit,
and framing-error behavior apply unchanged.

The first decoded octet identifies the message. Remaining decoded octets, if
any, are its payload. Layouts in this document are written before serial
escaping. This profile adds no generic length field or checksum.

Hexadecimal octets are written as two uppercase hexadecimal digits. Angle
brackets identify fields, for example `0A <mask_hi> <mask_lo>`. Multi-octet
unsigned integers are encoded most-significant octet first unless a layout
explicitly says otherwise.

## 3. Message and Response Model

Host-to-Device requests use one of the supported command identifiers in
Section 4. Device-to-host messages are synchronous responses to requests or
asynchronous CEC traffic and terminal results.

### 3.1 Generic Acceptance and Rejection

`COMMAND_ACCEPTED` has identifier `0x08`. A setter or action request accepted
synchronously produces exactly:

```text
08 <original_command_id>
```

`COMMAND_REJECTED` has identifier `0x09`. A validly framed request rejected by
the command layer produces exactly:

```text
09 <original_command_id>
```

`COMMAND_REJECTED` reports no detailed cause. A host MUST NOT expect an error
reason field.

Getter responses use the getter command identifier as their first octet and
the command-specific value as their remaining payload. TX terminal results and
asynchronous CEC RX messages use their own representations.

Framing-invalid input produces no message-layer response. Framing recovery is
defined by the serial binding and is distinct from command rejection.

### 3.2 Request Length

The request layouts below are exact. A supported command received with any
other decoded length MUST be rejected without performing that command's state
mutation or external action. Command-specific TX assembly preservation rules
are defined in Section 10.

## 4. Supported Command Registry

The following table is the complete host-to-Device command subset implemented
by this profile. The payload column excludes the command identifier.

| ID | Command | Request payload | Success response | State mutation | Possible CEC activity | Prerequisite |
|---:|---|---|---|---|---|---|
| `01` | `PING` | none | `08 01` | no | no | none |
| `0A` | `SET_ACK_MASK` | `u16be` | `08 0A` | ACK mask | future follower ACK | none |
| `0B` | `TRANSMIT` | one CEC octet | `08 0B` | TX assembly | no | assembling |
| `0C` | `TRANSMIT_EOM` | one CEC octet | `08 0C` | commits TX | yes, after acceptance | assembling and valid final frame |
| `0D` | `TRANSMIT_IDLETIME` | one octet | `08 0D` | TX compatibility state | no | TX idle |
| `0E` | `TRANSMIT_ACK_POLARITY` | boolean polarity | `08 0E` | starts TX assembly | no | TX idle |
| `15` | `FIRMWARE_VERSION` | none | `15 00 02` | no | no | none |
| `17` | `GET_BUILDDATE` | none | `17 6A 6E 88 80` | no | no | none |
| `18` | `SET_CONTROLLED` | boolean | `08 18` | controlled mode | future follower ACK | none |
| `1B` | `GET_DEFAULT_LOGICAL_ADDRESS` | none | `1B <address>` | no | no | none |
| `1C` | `SET_DEFAULT_LOGICAL_ADDRESS` | one address | `08 1C` | default logical address | no defined direct effect | none |
| `1D` | `GET_LOGICAL_ADDRESS_MASK` | none | `1D <mask_hi> <mask_lo>` | no | no | none |
| `1E` | `SET_LOGICAL_ADDRESS_MASK` | `u16be` | `08 1E` | logical-address mask | no defined direct effect | none |
| `1F` | `GET_PHYSICAL_ADDRESS` | none | `1F <pa_hi> <pa_lo>` | no | no | none |
| `20` | `SET_PHYSICAL_ADDRESS` | `u16be` | `08 20` | physical address | no defined direct effect | none |
| `28` | `GET_ADAPTER_TYPE` | none | `28 01` | no | no | none |

The ability of a configured ACK mask or controlled mode to affect a later CEC
acknowledgement is not an immediate CEC-bus action caused by the setter.

## 5. Basic Operation and Compatibility Metadata

### 5.1 PING (`0x01`)

The exact request is:

```text
01
```

The Device MUST respond with `08 01`. A request containing a payload MUST be
rejected as `09 01`. `PING` changes no state and causes no CEC-bus action.

### 5.2 Compatibility Metadata

The following requests and exact responses are fixed compatibility values:

| Command | Request | Response |
|---|---|---|
| `FIRMWARE_VERSION` | `15` | `15 00 02` |
| `GET_BUILDDATE` | `17` | `17 6A 6E 88 80` |
| `GET_ADAPTER_TYPE` | `28` | `28 01` |

These values are Pulse-Eight compatibility metadata. They MUST NOT be
interpreted as a CEC Injector firmware release version, source build timestamp,
Device Protocol version, or hardware-platform identity.

Each request has no payload. A request with another length MUST be rejected
with `COMMAND_REJECTED` for its command identifier.

## 6. Adapter Runtime State

Profile state is volatile and scoped to the transport attachment session unless
specified otherwise. Its safe defaults are:

| State | Default | Writer | Reader or observable effect |
|---|---:|---|---|
| controlled mode | false | `SET_CONTROLLED` | follower ACK eligibility |
| ACK mask | `0x0000` | `SET_ACK_MASK` | follower ACK destination selection |
| logical-address mask | `0x0000` | `SET_LOGICAL_ADDRESS_MASK` | `GET_LOGICAL_ADDRESS_MASK` |
| default logical address | `0x0F` | `SET_DEFAULT_LOGICAL_ADDRESS` | `GET_DEFAULT_LOGICAL_ADDRESS` |
| physical address | `0xFFFF` | `SET_PHYSICAL_ADDRESS` | `GET_PHYSICAL_ADDRESS` |
| TX idletime compatibility value | `0x03` | `TRANSMIT_IDLETIME` | associated with TX assembly, subject to Section 9.1 |
| TX assembly | idle and empty | TX commands | TX command acceptance and CEC TX |

Parser resynchronization and suspend/resume MUST preserve this state. The end
of the transport attachment session, Device reset, or power loss MUST reset it
to these defaults. This profile defines no persistent adapter storage.

## 7. Controlled Mode and Address State

### 7.1 SET_CONTROLLED (`0x18`)

The request is:

```text
18 <controlled>
```

`controlled` MUST be exactly `0x00` (false) or `0x01` (true). A valid request
sets controlled mode and produces `08 18`. Any other length or value produces
`09 18` and preserves the previous value.

Controlled mode gates the Device's follower acknowledgement behavior as
defined in Section 13. It does not define user-facing policy and does not
suppress asynchronous delivery of otherwise valid received CEC frames.

### 7.2 SET_ACK_MASK (`0x0A`)

The request is:

```text
0A <mask_hi> <mask_lo>
```

The mask is an unsigned 16-bit big-endian value. Every value is accepted. A
valid request stores the mask and produces `08 0A`.

Bit `N` selects CEC logical destination `N` for follower acknowledgement.
Destination `0x0F` is broadcast and is never acknowledged even if bit 15 is
set. The ACK mask is distinct from the logical-address mask.

### 7.3 Default Logical Address (`0x1B`, `0x1C`)

The getter request and response are:

```text
1B
1B <address>
```

The setter request is:

```text
1C <address>
```

`address` MUST be in the inclusive range `0x00` through `0x0F`. A valid setter
stores the value and produces `08 1C`.

This profile defines no additional behavioral effect for the stored default
logical address. In particular, it does not substitute for the ACK mask and
does not implicitly supply or rewrite a CEC TX initiator nibble.

### 7.4 Logical-Address Mask (`0x1D`, `0x1E`)

The getter request and response are:

```text
1D
1D <mask_hi> <mask_lo>
```

The setter request is:

```text
1E <mask_hi> <mask_lo>
```

The mask is an unsigned 16-bit big-endian value. Every value is accepted. A
valid setter stores the value and produces `08 1E`.

This profile defines no additional behavioral effect for the logical-address
mask. It does not control follower ACK behavior; `SET_ACK_MASK` controls that
behavior independently.

### 7.5 Physical Address (`0x1F`, `0x20`)

The getter request and response are:

```text
1F
1F <pa_hi> <pa_lo>
```

The setter request is:

```text
20 <pa_hi> <pa_lo>
```

The physical address is an unsigned 16-bit big-endian value. Every value from
`0x0000` through `0xFFFF` is accepted. A valid setter stores the value and
produces `08 20`.

This is Pulse-Eight-profile runtime state. This profile defines no topology
validation and no additional direct behavior for it. An extension may define
separate physical-address state with its own validation without changing this
field.

## 8. CEC TX Transaction Model

At most one TX transaction exists. Its abstract states are:

- **TX idle**: no transaction is being assembled or delivered;
- **assembling**: polarity is set and zero or more CEC octets are collected;
- **committed**: a valid final octet was accepted and the CEC frame awaits
  physical transmission;
- **in progress**: physical transmission has begun; and
- **terminal pending**: physical transmission completed and one asynchronous
  terminal result awaits delivery.

A CEC frame contains from 1 through 16 octets inclusive. TX commands do not add,
remove, or rewrite CEC header fields. The first assembled octet is the CEC
header and its low nibble is the destination.

`TRANSMIT_ACK_POLARITY` starts assembly. `TRANSMIT` appends a non-final octet.
`TRANSMIT_EOM` appends the final octet and commits a valid frame. Physical CEC
transmission MUST NOT begin before the Device has made the synchronous
`COMMAND_ACCEPTED` response for the committing `TRANSMIT_EOM` available to the
transport.

An assembling transaction expires 1,000,000 microseconds after the most recent
accepted assembly start, accepted appended octet, or rejected
polarity-mismatched EOM attempt. Expiry resets TX state to idle and the idletime
compatibility value to `0x03`. It produces no TX terminal result.

After a terminal result is delivered, TX state returns to idle and the idletime
compatibility value returns to `0x03`.

## 9. TX Commands

### 9.1 TRANSMIT_IDLETIME (`0x0D`)

The request is:

```text
0D <value>
```

Any one-octet value is legal. The command is accepted only in TX idle state. A
valid request stores the compatibility value and produces `08 0D`.

The stored value is associated with the transaction subsequently started by
`TRANSMIT_ACK_POLARITY`. It remains associated through assembly and physical
transmission. It resets to the profile default `0x03` on assembly timeout,
terminal delivery, session reset, Device reset, or power loss.

This command is **implemented with a documented limitation**: acceptance and
storage of the value do not guarantee that the host-selected value controls
physical CEC bus-idle timing. A conforming implementation of this profile MUST
provide the frozen acceptance, state, and reset behavior, but this profile does
not define a mapping from the value to physical timing.

### 9.2 TRANSMIT_ACK_POLARITY (`0x0E`)

The request is:

```text
0E <polarity>
```

`polarity` MUST be `0x00` for a directly addressed frame or `0x01` for a
broadcast frame. The command is accepted only in TX idle state. Acceptance
starts an empty assembling transaction and produces `08 0E`.

At commit, direct polarity requires a destination nibble other than `0x0F`.
Broadcast polarity requires destination `0x0F`.

### 9.3 TRANSMIT (`0x0B`)

The request is:

```text
0B <cec_octet>
```

The command is accepted only while assembling and while fewer than 16 octets
have been collected. Acceptance appends one non-final CEC octet, refreshes the
assembly deadline, and produces `08 0B`. It does not begin physical CEC
transmission.

### 9.4 TRANSMIT_EOM (`0x0C`)

The request is:

```text
0C <cec_octet>
```

The command is accepted only while assembling and while fewer than 16 octets
have been collected. It tentatively appends the final octet and validates the
destination nibble of the first assembled octet against the selected polarity.

If valid, the complete frame is committed and the Device produces `08 0C`.
The acceptance is exposed before physical CEC transmission begins. Physical
completion is later reported by exactly one asynchronous terminal result if
the attachment session remains available.

If polarity validation fails, the tentative EOM octet is removed, `09 0C` is
produced, and the earlier partial assembly remains active with its refreshed
deadline.

## 10. TX Validation and Rejection

Every TX command request MUST contain exactly one payload octet. A wrong-length
request is rejected without changing TX assembly.

| Current state or condition | `0D` idletime | `0E` polarity | `0B` segment | `0C` EOM |
|---|---|---|---|---|
| TX idle | accept | accept legal `00`/`01` | reject | reject |
| assembling, length below 16 | reject | reject | accept | accept if polarity matches header |
| assembling, length 16 | reject | reject | reject | reject |
| committed, in progress, or terminal pending | reject | reject | reject | reject |
| invalid polarity value | not applicable | reject | not applicable | not applicable |
| EOM/header polarity mismatch | not applicable | not applicable | not applicable | reject and remove only tentative EOM |

Unless the mismatch rule explicitly removes the tentative EOM octet, rejection
of a TX command preserves the existing partial assembly and its existing
deadline. A validly framed unsupported command or a supported non-TX command
does not cancel TX assembly. A supported non-TX command is processed according
to its own rules while assembly remains active.

## 11. TX Terminal Results

After physical transmission completes, the Device produces one of these exact
one-octet asynchronous messages:

| ID | Terminal result | Meaning in this profile |
|---:|---|---|
| `10` | `TRANSMIT_SUCCEEDED` | transmission completed successfully |
| `11` | `TRANSMIT_FAILED_LINE` | line, arbitration, disabled-transmitter, or internal failure |
| `12` | `TRANSMIT_FAILED_ACK` | expected acknowledgement was not observed |
| `13` | `TRANSMIT_FAILED_TIMEOUT_DATA` | data transmission timed out |
| `14` | `TRANSMIT_FAILED_TIMEOUT_LINE` | bus remained unavailable or busy |

A terminal result is asynchronous and may be delayed relative to EOM
acceptance. At most one terminal result is delivered for a committed
transaction. Representability of a result does not require every physical
transmitter to expose a distinct internal cause for it.

Suspend/resume within the same attachment session preserves a pending terminal
result, which MUST be delivered at most once when communication permits. If the
attachment session ends before delivery, the pending transaction and result
MUST be discarded. They MUST NOT be replayed in a later session.

## 12. Asynchronous CEC RX

Each received CEC octet is delivered as one decoded two-octet message:

```text
<message_id_and_flags> <cec_octet>
```

The first CEC octet uses base identifier `0x05` (`FRAME_START`). Every later
CEC octet uses base identifier `0x06` (`FRAME_DATA`). The following flag bits
are ORed into that identifier:

| Bit | Value | Name | Meaning |
|---:|---:|---|---|
| 6 | `0x40` | `ACK` | on the header message, the observed header ACK slot was physically high (NACK) |
| 7 | `0x80` | `EOM` | this message carries the final CEC octet |

The `EOM` flag appears only on the final octet. For a one-octet CEC frame, the
first message also carries `EOM`.

The `ACK` flag is meaningful only on the first, header message. It is set when
the observed header ACK slot was physically high, meaning no acknowledgement
was observed. It is clear when the slot was physically low. This
Pulse-Eight-compatible naming is intentionally counterintuitive relative to an
ordinary positive Boolean reading of “ACK.” Operand messages MUST NOT carry
the `ACK` flag.

Messages preserve CEC octet order. A received frame is delivered only if it is
complete, has a valid final EOM indication, contains 1 through 16 octets, has
no receive error, did not overlap local TX, and was not affected by receive
analysis overflow. If the complete set of messages cannot be queued, the
Device MUST suppress the whole CEC frame rather than expose a truncated prefix.
Suppression produces no synthetic error message.

## 13. Controlled Follower Acknowledgement

For a valid directly addressed CEC frame, the Device acts as a follower for the
destination only when all of these conditions hold at the start of that CEC
frame:

- controlled mode is true;
- the ACK-mask bit corresponding to the destination is set; and
- CEC receive processing is available.

Broadcast destination `0x0F` MUST NOT be acknowledged. A disabled controlled
mode or an unset ACK-mask bit MUST leave the Device passive for that
destination.

When the conditions hold, the Device supplies the follower acknowledgement for
each correctly received CEC octet in that frame, including the header. This
follower action is independent of the `ACK` flag used to report the observed
header ACK slot in Section 12.

The Device snapshots controlled mode and the ACK mask at the valid CEC frame
start. Changes made after that start apply to a subsequent frame and MUST NOT
alter the acknowledgement decision for the frame already being received.

Malformed, truncated, timed-out, or overflow-affected reception MUST release
any prepared follower acknowledgement and MUST NOT leave the CEC line actively
driven. The logical-address mask and default logical address do not substitute
for the ACK mask.

## 14. Unsupported Commands

A non-empty, validly framed host request whose command identifier is not in
Section 4 MUST produce:

```text
09 <original_command_id>
```

The request MUST NOT cause its associated external action and MUST NOT reset
unrelated profile state or active TX assembly. This rule includes recognized
Pulse-Eight commands outside the documented subset and unknown command
identifiers.

This profile does not require unsupported getters or modes for Linux kernel
`pulse8-cec` compatibility. Project extensions are specified separately and
are not classified by this section when an implementation claims them.

## 15. Attachment Lifecycle

The transport attachment lifecycle in the main Device Protocol specification
applies to this profile.

Parser resynchronization MUST preserve adapter and TX state. Suspend/resume
alone MUST NOT reset runtime adapter state, partial TX assembly, committed or
in-progress TX state, or a pending terminal result. The ordinary assembly
timeout continues to apply while suspended. Suspend does not authorize a new TX
operation and resume does not synthesize or duplicate a result.

Ending the transport attachment session MUST reset adapter state to Section 6
defaults and abandon partial, committed, in-progress, or terminal-pending
host-visible TX state. A later session MUST NOT receive a terminal result from
the ended session.

## 16. Known Compatibility Limitations

The following limitations are part of the profile claim:

- compatibility is limited to the commands and behavior in this document;
- `TRANSMIT_IDLETIME` is accepted and stored but does not define or guarantee
  host-selected physical bus-idle timing;
- default logical address, logical-address mask, and physical address are
  stored and readable but have no additional behavior defined by this profile;
- no persistent adapter configuration is defined;
- no automatic discovery, USB identity, or kernel-driver compatibility is
  claimed; and
- generic rejection does not carry a detailed error cause.

These limitations MUST NOT be represented as full implementation of omitted
Pulse-Eight behavior.

## 17. Validation Status (Non-Normative)

The reference implementation has been validated for explicit-port
interoperation with libCEC 7.1.1. This validation target is evidence for the
profile; libCEC is not a normative dependency or conformance requirement.

The reference implementation uses an ordered USB CDC byte stream as a carrier.
USB CDC, USB identity, endpoint layout, operating system, and hardware platform
are not requirements of this profile.
