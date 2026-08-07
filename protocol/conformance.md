# CEC Injector Device Protocol v1 Conformance

**Part of CEC Injector Device Protocol v1**

**Maturity: Experimental**

## Status

This document normatively defines conformance claims for CEC Injector Device
Protocol v1. It does not define wire semantics and does not certify products or
implementations.

## 1. Conformance and Validation

**Conformance** means that an implementation satisfies the normative
requirements of every protocol part for which it claims support.

**Validation** is evidence obtained by testing a particular implementation in
a particular environment. Validation may support a conformance claim, but its
hardware, software, transport carrier, operating system, and attached equipment
do not become protocol requirements.

Conformance MUST be testable through externally observable protocol I/O and
lifecycle stimuli. Source-code inspection MUST NOT be required to establish
conformance.

## 2. Claim Identification

A conformance claim MUST identify:

- CEC Injector Device Protocol version;
- every transport binding claimed;
- every compatibility profile claimed;
- every extension and extension version claimed;
- whether the claim applies to a Device or a host; and
- any explicitly permitted subset, if its defining document allows subset
  claims.

An implementation MUST NOT use an unqualified compatibility statement that
could reasonably be interpreted as support for an unimplemented binding,
profile, extension, or role.

## 3. Device Protocol v1 Claim

The claim **CEC Injector Device Protocol v1** means that the implementation
obeys the platform-neutral requirements of
[the main specification](specification.md) for everything it claims to
implement.

This claim alone does not imply support for every v1 transport binding,
compatibility profile, or extension. It also does not imply a particular
hardware platform, transport carrier, host implementation, or persistent
configuration mechanism.

## 4. Component Claims

### 4.1 Pulse-Eight Serial Transport Binding

The claim **Pulse-Eight Serial Transport Binding** requires complete conformance
to [the binding](pulse8-serial-binding.md), including canonical encoding,
streaming across arbitrary delivery boundaries, malformed-frame handling,
resynchronization, decoded-frame limits, and attachment-session interaction.

The binding does not by itself imply any decoded-message command set.

### 4.2 Pulse-Eight Compatibility Profile

The claim **Pulse-Eight Compatibility Profile** requires the complete supported
subset defined by [the profile](pulse8-compatibility-profile.md), including all
listed commands, responses, state, CEC TX/RX behavior, follower ACK behavior,
known limitations, and lifecycle rules.

A partial implementation MUST NOT claim the complete Pulse-Eight Compatibility
Profile. A narrower implementation may describe its implemented commands, but
that description is not this profile's conformance claim.

The profile claim does not imply full upstream Pulse-Eight compatibility,
automatic discovery, or compatibility with an unlisted host or driver.

### 4.3 Wake Extension v1

The claim **Wake Extension v1** requires complete conformance to
[the extension](wake-extension-v1.md), including all commands `0xE0` through
`0xE7`, exact state and lifecycle behavior, mode 1 matching, diagnostics,
duplicate suppression, and reset behavior.

The claim does not imply a specific hardware or transport wake mechanism. The
claimed binding or carrier MUST provide the lifecycle, authorization, request,
and failure mapping required by the extension.

An implementation providing only part of E0–E7 MUST NOT claim Wake Extension
v1.

## 5. Composition

Claims compose only when every claimed component's dependencies are also met.
For example, a Device may claim:

```text
CEC Injector Device Protocol v1
+ Pulse-Eight Serial Transport Binding
+ Pulse-Eight Compatibility Profile
+ Wake Extension v1
```

That combination describes the initial complete protocol set, but it is not
mandatory for every future Device. Another Device Protocol v1 implementation
may claim a different defined binding or omit an optional profile or extension.

An extension is not implicitly dependent on the Pulse-Eight Compatibility
Profile merely because the initial extension uses its response model. Each
extension's normative dependencies are those stated in its own document.

## 6. Device Conformance Areas

A Device claim MUST be evaluated in every applicable observable area:

| Area | Observable subject |
|---|---|
| Framing | encoding, decoding, streaming, limits, errors, and resynchronization |
| Pulse-Eight profile | requests, responses, runtime state, CEC TX/RX, and follower ACK |
| Wake Extension | capabilities, E0–E7, matcher, delivery, diagnostics, and duplicate suppression |
| Lifecycle | attach, parser recovery, suspend, resume, detach, reset, and power loss |
| Malformed and unsupported input | silence at framing layer, command rejection, and non-destructive failure |
| Session and reset | safe defaults, state lifetime, and absence of stale action/result replay |

Internal module structure, programming language, MCU, transport library, and
electrical implementation are not conformance evidence or requirements.

## 7. Host Conformance

A host MAY claim only the protocol portions it actually implements. A host does
not implement Device-owned delivery mechanisms, but for every claimed part it
MUST:

- emit conforming requests and encodings;
- decode conforming responses and asynchronous messages;
- interpret fields, flags, enums, diagnostics, and lifecycle behavior according
  to the normative documents;
- distinguish framing silence from command rejection;
- obey stated prerequisites and forward-compatibility rules; and
- avoid treating validation-specific behavior as normative.

A host that uses only selected operations MAY state that limited support, but
MUST NOT claim a complete profile or extension where the corresponding claim
requires the whole component.

## 8. Observable Test Boundary

Applicable conformance checks MUST be expressible using:

- bytes supplied to and observed from a claimed binding;
- CEC frames supplied to or observed from the Device;
- binding lifecycle, authorization, reset, and attachment stimuli;
- observable remote-wake requests and reported outcomes; and
- passage of specified protocol time intervals.

Tests MAY use implementation-specific fixtures to create those stimuli, but a
requirement that can be established only by inspecting private implementation
state is not a valid conformance criterion.

Diagnostic counters and timing fields are tested according to their specified
observable updates and representation. They MUST NOT be promoted to
control-authoritative state by a conformance test.

## 9. Validation Scope (Non-Normative)

RP2040 hardware, Philips 55PUS9010/12, and explicit-port libCEC 7.1.1 describe
validation environments of the reference implementation. They are not
requirements for Device or host conformance.
