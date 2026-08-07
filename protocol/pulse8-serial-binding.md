# Pulse-Eight Serial Transport Binding

**Part of CEC Injector Device Protocol v1**

**Maturity: Experimental**

## Status

This document is normative within the scope of the Pulse-Eight Serial Transport
Binding. It defines how an ordered octet stream is converted to decoded protocol
frames. The maturity and versioning rules of [CEC Injector Device Protocol
v1](specification.md) apply.

## 1. Scope

This binding defines:

- framing over an ordered octet stream;
- reserved framing octets and escaping;
- encoded and decoded frame boundaries;
- parser resynchronization;
- malformed-frame handling;
- the decoded-frame size limit; and
- independence from carrier delivery boundaries.

This binding does not define the meaning of any decoded frame, operation,
request, response, result, or extension. It does not define HDMI-CEC electrical
behavior, a hardware platform, transport descriptors, endpoint layout, or
transport identity.

The terms and general lifecycle requirements in the Device Protocol v1
specification apply to this binding.

## 2. Transport Model

This binding operates over an **ordered octet stream**. An octet is an unsigned
eight-bit value.

A conforming carrier MUST preserve octet order and MUST deliver every
successfully transferred octet exactly once to the framing layer. Carrier
delivery boundaries MAY occur at arbitrary positions relative to encoded frame
boundaries.

One carrier read or write need not correspond to one frame. Packet boundaries,
when present in a carrier, have no framing significance under this binding.

## 3. Terminology

**transport octet**
: One octet delivered by the ordered stream to the framing layer.

**encoded frame**
: The sequence beginning with a frame-start delimiter and ending with a valid
  frame-end delimiter, including any escape introducers and escape codes.

**decoded frame**
: The non-empty sequence of one to 64 octets obtained after the delimiters are
  removed and every valid escape sequence is decoded.

**frame-start delimiter**
: The unescaped reserved octet `FF`, named `FRAME_START_DELIMITER` in this
  binding.

**frame-end delimiter**
: The unescaped reserved octet `FE`, named `FRAME_END_DELIMITER` in this
  binding.

**escape introducer**
: The reserved octet `FD`, named `FRAME_ESCAPE` in this binding, when received
  inside a frame.

**escape code**
: The single octet following an escape introducer. Only `FA`, `FB`, and `FC`
  are valid escape codes.

**parser idle state**
: The state in which no frame is being accumulated.

**parser in-frame state**
: The state after a frame-start delimiter in which decoded octets are being
  accumulated.

**parser escape-pending state**
: The state after an escape introducer in which exactly one escape code is
  expected.

**framing error**
: An empty frame, invalid escape sequence, decoded-frame overflow, or incomplete
  frame discarded by resynchronization.

**resynchronization**
: Abandoning any incomplete or malformed framing context and establishing a new
  frame boundary at a subsequently received frame-start delimiter.

These parser states are specification vocabulary. An implementation MAY use any
internal parser architecture that produces the same observable behavior.

## 4. Reserved Framing Octets

| Octet | Name | Binding function |
|---:|---|---|
| `FF` | `FRAME_START_DELIMITER` | Begins a frame and unconditionally resynchronizes the parser. |
| `FE` | `FRAME_END_DELIMITER` | Ends a frame when unescaped in the in-frame state. |
| `FD` | `FRAME_ESCAPE` | Introduces one escape code in the in-frame state. |

All three octets are reserved framing values when unescaped.

`FRAME_START_DELIMITER` MUST begin a new encoded frame. It is an unconditional
resynchronization boundary in every parser state. If another frame is
incomplete, that frame MUST be discarded and a new frame MUST begin immediately
at this delimiter.

`FRAME_END_DELIMITER` terminates a frame only when received unescaped in the
in-frame state. It is not part of the decoded frame. Outside a frame it is
ignored. In the escape-pending state it is an invalid escape code and causes the
affected frame to be discarded.

`FRAME_ESCAPE` introduces exactly one escape code when received in the in-frame
state. It is not part of the decoded frame. Outside a frame it is ignored. In
the escape-pending state it is an invalid escape code and causes the affected
frame to be discarded.

## 5. Canonical Encoding

The canonical encoded representation is:

```text
FF <escaped decoded octets> FE
```

A conforming sender MUST begin each encoded frame with
`FRAME_START_DELIMITER`, encode every decoded octet as specified in Section 6,
and end the frame with `FRAME_END_DELIMITER`.

The framing layer adds no checksum, cyclic redundancy check, sequence number,
transport identifier, explicit encoded-length field, or decoded-length field.
Decoded length is the number of octets accumulated after valid escape processing
between the delimiters.

## 6. Escaping

The complete escape mapping is:

| Decoded octet | Encoded octets |
|---:|---:|
| `FD` | `FD FA` |
| `FE` | `FD FB` |
| `FF` | `FD FC` |

A conforming sender MUST use these mappings for every decoded `FD`, `FE`, and
`FF`. Every other decoded octet MUST be emitted unchanged between the frame
delimiters.

A conforming sender MUST NOT emit a reserved framing octet literally as decoded
content. No alternative escape mapping is valid.

After `FRAME_ESCAPE`, a conforming receiver MUST decode `FA` as `FD`, `FB` as
`FE`, and `FC` as `FF`. Any other escape code is invalid and causes the entire
affected frame to be discarded.

## 7. Decoded-Frame Limits

A valid decoded frame contains from one to 64 decoded octets inclusive. This
limit is a property of this binding, not a universal limit of CEC Injector
Device Protocol v1 or another transport binding.

A frame terminated with no decoded octets is invalid and MUST be discarded.

The receiver MUST discard a frame as soon as processing an ordinary octet or a
valid escape sequence would append a 65th decoded octet. The first 64 octets of
an oversized frame MUST NOT be delivered as a partial decoded frame. After
overflow, subsequent octets are ignored until a new
`FRAME_START_DELIMITER` establishes a new frame.

## 8. Streaming and Delivery Boundaries

Framing is independent of carrier delivery boundaries. A conforming receiver
MUST correctly process:

- a complete encoded frame delivered in one carrier operation;
- a frame split at any octet boundary across multiple operations;
- multiple complete frames delivered in one operation;
- a frame end followed immediately by another frame start; and
- an escape introducer and its escape code delivered in separate operations.

An incomplete frame at the end of a carrier delivery remains pending. A
delivery boundary is not a framing error, does not imply a frame end, and does
not require a flush.

## 9. Parser and Resynchronization Model

### 9.1 IDLE

In `IDLE`, all octets other than `FRAME_START_DELIMITER` are ignored.
`FRAME_START_DELIMITER` clears the decoded-frame accumulator and enters
`IN_FRAME`.

### 9.2 IN_FRAME

In `IN_FRAME`:

- an ordinary unreserved octet appends one decoded octet, subject to the
  64-octet limit;
- `FRAME_ESCAPE` enters `ESCAPE_PENDING`;
- `FRAME_END_DELIMITER` completes the frame if at least one decoded octet has
  been accumulated;
- `FRAME_END_DELIMITER` with no decoded octets causes an empty-frame error; and
- `FRAME_START_DELIMITER` discards the incomplete frame, clears the accumulator,
  and immediately begins a new frame in `IN_FRAME`.

After a valid frame is completed, the parser returns to `IDLE`.

### 9.3 ESCAPE_PENDING

In `ESCAPE_PENDING`:

- `FA`, `FB`, and `FC` append their mapped decoded octet, subject to the
  64-octet limit, and return the parser to `IN_FRAME`;
- `FRAME_START_DELIMITER` discards the incomplete frame, clears the accumulator,
  and immediately begins a new frame in `IN_FRAME`; and
- every other octet, including `FRAME_END_DELIMITER` and `FRAME_ESCAPE`, causes
  the affected frame to be discarded.

After an invalid escape code or decoded-frame overflow, the receiver ignores
input until a subsequent `FRAME_START_DELIMITER` begins a new frame.

## 10. Malformed Input

| Input condition | Required observable behavior | Recovery |
|---|---|---|
| Octet outside a frame | Ignore it. | Remain idle until `FF`. |
| Carrier delivery ends during a frame | Preserve the incomplete framing state; this is not an error. | Continue with the next delivered octet. |
| `FF FE` | Discard the empty frame. | Ignore input until a later `FF`. |
| `FD` followed by `FA`, `FB`, or `FC` inside a frame | Decode the defined reserved octet; this is valid input. | Continue the same frame. |
| `FD` followed by any other octet except `FF` | Discard the affected frame. `FE` and `FD` are invalid in this position. | Ignore input until a later `FF`. |
| `FD FF` inside a frame | Discard the incomplete frame; treat `FF` as the start of a new frame. | Continue in the new frame. |
| Unescaped `FF` during an incomplete frame | Discard the incomplete frame and immediately begin a new frame. | Continue in the new frame. |
| Attempt to append decoded octet 65 | Discard the entire oversized frame. | Ignore input until a later `FF`. |
| Frame remains incomplete without further input | Deliver no decoded frame and retain parser state while the attachment remains available. | Continue when more octets arrive, or discard on session end. |

No framing-invalid input may be delivered to the protocol-message layer as a
partial or similar valid decoded frame. A valid frame received after the stated
recovery boundary MUST remain processable.

## 11. Error Visibility

Framing errors are silent at the protocol-message layer. Empty frames, invalid
escape sequences, oversized frames, and incomplete frames abandoned by
resynchronization produce no decoded frame and no protocol response solely
because of the framing error.

Rejection of a validly framed but semantically invalid decoded message belongs
to the compatibility profile or extension that defines that message. It is not
a framing action and is outside this document's scope.

## 12. Attachment Session Interaction

Parser state and protocol session state are distinct. Parser resynchronization
MUST NOT by itself end the transport attachment session or reset session-scoped
Device configuration. A new `FRAME_START_DELIMITER` during an attachment
session resynchronizes framing only.

Malformed input MUST NOT by itself end the attachment session. When the
attachment session ends, the receiver MUST discard any binding-local partial
frame and escape-pending state. General attachment lifecycle rules are defined
in Sections 7 and 8 of the Device Protocol v1 specification.

## 13. Examples

**NON-NORMATIVE**

### 13.1 Simple Frame

```text
decoded: 11 22
encoded: FF 11 22 FE
```

### 13.2 Reserved Octets

```text
decoded: FD FE FF
encoded: FF FD FA FD FB FD FC FE
```

### 13.3 Split Delivery

```text
delivery 1: FF 11 FD
delivery 2: FA 22 FE
decoded:    11 FD 22
```

The escape introducer remains pending across the delivery boundary.

### 13.4 Resynchronization

```text
stream:  FF 11 22 FF 33 FE
discard:    11 22
decoded:             33
```

The second unescaped `FF` abandons the incomplete frame and begins the frame
that decodes to `33`.

## 14. Validation Status

**NON-NORMATIVE**

The reference implementation has been validated with an ordered USB CDC byte
stream. USB CDC is not required by the platform-neutral Device Protocol or by
the abstract ordered-stream model of this binding. USB identity is not part of
this binding. Endpoint packet size does not define protocol frame size, and USB
transfer boundaries do not define frame boundaries.
