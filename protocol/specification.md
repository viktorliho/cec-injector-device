# CEC Injector Device Protocol v1

**Maturity: Experimental**

## Status

CEC Injector Device Protocol v1 is a published, implementable protocol contract.
Publication freezes the meaning of v1, but does not make the protocol Stable or
claim that any implementation or hardware design is production-ready.

Experimental maturity means that future CEC Injector protocols may make
incompatible changes. Any breaking wire change MUST use a new Device Protocol
version. Version 1 MUST NOT be redefined retroactively. Future project releases
MAY stop supporting v1, but the published meaning of v1 remains unchanged.

Device Protocol v1 has no required wire-level Device Protocol version field.
The Device Protocol version and independently versioned extensions are separate
concepts.

## 1. Scope

This specification defines the observable contract between a CEC Injector
Device and a host. It covers:

- platform-neutral Device and host responsibilities;
- transport bindings and compatibility profiles declared by v1;
- protocol-visible state and lifecycle behavior;
- CEC Injector project extensions; and
- terminology for compatibility and conformance claims.

This specification does not define user-facing host policy, CEC Injector Mate
architecture, a particular operating system or host service, implementation
internals, electrical or mechanical designs, HDMI certification requirements,
or television-vendor behavior. A transport binding may depend on a particular
transport without making that transport a universal Device requirement.

Implementation and validation notes may accompany the normative contract, but
they do not extend it unless the specification explicitly says otherwise.

## 2. Normative Language

The key words **MUST**, **MUST NOT**, **REQUIRED**, **SHALL**, **SHALL NOT**,
**SHOULD**, **SHOULD NOT**, **RECOMMENDED**, **NOT RECOMMENDED**, **MAY**, and
**OPTIONAL** in this document are to be interpreted as described in RFC 2119 and
RFC 8174 when, and only when, they appear in all capitals.

Definitions state the meaning of terms and are normative where that meaning is
required to interpret a requirement, even when they do not contain an uppercase
key word.

## 3. Architectural Roles

### 3.1 Device

The **Device** is a programmable HDMI-CEC endpoint. It owns the capabilities and
delivery mechanisms that it exposes through the protocol.

The Device MAY perform bounded autonomous work necessary to deliver an intent
that has already been defined by the host or by an explicit protocol contract.
Such work MUST preserve the meaning and authority of that intent. The Device
MUST NOT invent user-facing host intent.

An externally observable capability MUST have explicit prerequisites and
lifecycle behavior. Delivery work performed without active host software MUST
remain within those prerequisites.

### 3.2 Host

The **host** is the system attached to the Device through a supported transport
binding. It originates protocol configuration and operational intent and
consumes Device capabilities and observations.

The host MUST follow the requirements of every binding, compatibility profile,
and extension for which it claims support. This role does not assume a
particular operating system, software library, application model, or user
interface.

### 3.3 Mate

**CEC Injector Mate** is the project's production host-software role. Mate owns
user-facing host policy and may act as an ordinary protocol host.

Mate is not required to implement or use the Device Protocol. A third-party host
implementation may interoperate with a Device without Mate. Mate behavior is
outside Device Protocol conformance except for the protocol behavior it exposes
while acting as a host.

## 4. Protocol Model

CEC Injector Device Protocol v1 is composed of the following conceptual layers:

1. a platform-neutral Device contract and common vocabulary;
2. transport bindings and compatibility profiles that carry concrete protocol
   interactions;
3. the Pulse-Eight Compatibility Profile carried by the initial serial
   binding;
4. the CEC Injector project extension namespace;
5. Wake Extension v1;
6. lifecycle and transport attachment session rules; and
7. conformance profiles for supported combinations of these parts.

A transport binding defines how interactions are represented and delivered on
a particular transport. A compatibility profile defines a specific compatible
subset or interpretation of an existing protocol. An extension adds declared
capabilities without changing existing meanings.

An implementation MAY support an explicitly identified subset when the
applicable conformance rules allow it. A subset claim MUST identify the
supported binding, profile, and extensions, and MUST preserve the semantics of
every supported part. Omitted behavior MUST NOT be presented as implemented.

The platform-neutral contract does not require all implementations to use the
initial binding or profile.

## 5. Versioning

### 5.1 Device Protocol Version

The name **CEC Injector Device Protocol v1** identifies this published
observable contract. V1 does not require a Device Protocol version value in
each message or any other wire-level Device Protocol version field.

An implementation identifies v1 through its documented compatibility claim and
selection of the applicable v1 binding, profile, and extensions. A future
incompatible Device Protocol MUST receive a different Device Protocol version.

### 5.2 Extension Versions

Extensions are versioned independently of the Device Protocol. An extension
MAY define an on-wire version field. Such a field identifies the extension
contract specified for that field; it does not identify the Device Protocol or
an implementation release.

Wake Extension version 1 is an independently versioned v1 extension. Its wire
format and behavior are defined in [Wake Extension v1](wake-extension-v1.md).

### 5.3 Implementation Versions

Firmware, software, hardware revision, and release versions are implementation
metadata. They are not Device Protocol versions and MUST NOT be used as a
substitute for a protocol compatibility claim.

### 5.4 Compatibility Metadata

A compatibility profile may expose legacy or profile-specific metadata required
for interoperability with another protocol. Unless explicitly defined
otherwise, such metadata MUST NOT be interpreted as a CEC Injector Device
Protocol version or as CEC Injector firmware release metadata.

## 6. Terminology

**Device**
: The programmable HDMI-CEC endpoint defined in Section 3.1.

**host**
: The system communicating with the Device through a supported transport
  binding, as defined in Section 3.2.

**Mate**
: The optional production host-software role defined in Section 3.3.

**intent**
: A defined request or authorization for an externally meaningful operation.
  Intent originates outside Device-owned delivery logic unless an explicit
  protocol contract already defines it.

**capability**
: A declared operation or delivery mechanism that a Device can provide under
  defined prerequisites.

**delivery mechanism**
: Device-owned behavior used to carry out already-defined intent or deliver an
  observation, including bounded state tracking, retry, or duplicate
  suppression where explicitly specified.

**transport binding**
: A specification of how protocol interactions are framed, delivered, bounded,
  and mapped to lifecycle events on a concrete transport.

**compatibility profile**
: A declared set of behaviors provided for interoperability with another
  protocol or ecosystem. A profile claim applies only to its documented subset.

**extension**
: An independently identified and, where required, independently versioned set
  of additional capabilities that does not redefine existing base semantics.

**transport attachment session**
: The interval beginning when the active transport binding reports the Device
  as available for protocol communication and ending on binding-level detach,
  unmount, binding reset, Device reset, or power loss. Suspend and resume alone
  do not begin a new transport attachment session.

**attach**
: The binding-level event that makes the Device available and begins a transport
  attachment session.

**detach**
: A binding-level loss of attachment that ends the current transport attachment
  session.

**suspend**
: A binding-reported temporary state in which ordinary host communication may
  be unavailable or restricted without ending the transport attachment session.

**resume**
: The binding-reported return from suspend within the same transport attachment
  session.

**reset**
: An event that returns the relevant protocol or binding component to its
  defined initial state. Sections defining a reset MUST state whether it ends
  the transport attachment session.

**parser resynchronization**
: Recovery of message-boundary parsing after incomplete or malformed transport
  input without, by itself, ending the transport attachment session.

**externally observable behavior**
: Protocol output, state visible through protocol behavior, transport-visible
  lifecycle behavior, HDMI-CEC bus activity, host wake signaling, or another
  effect visible outside an implementation.

**normative behavior**
: Behavior required for compatibility by the normative portions of this
  specification.

**diagnostic state**
: Observable information intended for inspection or troubleshooting. Its exact
  value is not automatically a valid control input and is not
  control-authoritative unless the section defining it explicitly says
  otherwise.

## 7. State Ownership

Every state item exposed through protocol behavior MUST have a defined owner
and lifetime. Later binding, profile, and extension sections define the owners,
initial values, mutations, and reset rules for their specific state.

Host-configurable state MUST change only through a defined protocol operation
or a defined lifecycle reset rule. Device-owned state MAY change in response to
external or lifecycle events only where the protocol defines that relationship.

Parser resynchronization alone MUST NOT start a new transport attachment
session. A Device MUST NOT reset session-scoped state merely because its parser
discarded incomplete or malformed input.

Suspend and resume alone MUST NOT reset session-scoped state. Session-scoped
volatile state MUST NOT survive the end of a transport attachment session
unless a particular extension explicitly defines persistence.

V1 defines no general persistent-configuration mechanism. This does not
prohibit a later section or future extension from defining a specifically
scoped persistent state contract.

A new transport attachment session MUST NOT replay an action solely because
that action or its result was pending in an earlier session.

## 8. Transport Attachment Lifecycle

### 8.1 Initial and Reset State

On initialization or a reset that ends the attachment session, the Device MUST
enter the safe defaults defined by each supported profile and extension.
Externally observable operations or results from an earlier session MUST NOT be
replayed.

### 8.2 Attachment

An attach event begins a new transport attachment session. The host may then
discover capabilities and configure state provided by its selected binding,
profile, and extensions.

Attachment alone MUST NOT be interpreted as new operational intent and MUST NOT
replay an operation from a previous session.

### 8.3 Parser Recovery

Malformed or incomplete transport input may cause parser resynchronization as
defined by the active binding. Parser resynchronization does not by itself end
the attachment session and MUST NOT reset session-scoped configuration.

Discarded malformed input MUST NOT be reconstructed as a similar valid
operation.

### 8.4 Suspend

A binding may expose suspend. Suspend alone does not end the transport
attachment session. Session-scoped configuration MUST be preserved unless a
specific profile or extension explicitly defines a different lifetime for a
particular state item.

Communication and delivery behavior while suspended are defined by the active
binding and applicable extensions.

### 8.5 Resume

Resume continues the existing transport attachment session. State preserved
through suspend remains in effect, and resume alone MUST NOT replay an operation
that was already completed or abandoned.

### 8.6 Detach and Binding Reset

Detach, unmount, or a binding reset that invalidates attachment ends the current
transport attachment session. Session-scoped volatile state MUST return to the
safe defaults defined for the supported profile and extensions.

Pending actions and pending results from the ended session MUST NOT be replayed
into a later session unless a future specification explicitly defines a
persistent operation with different rules.

### 8.7 Device Reset and Power Loss

Device reset and power loss end the transport attachment session and clear
volatile protocol state. After availability returns, the next attachment is a
new session. V1 does not imply the existence of persistent storage.

## 9. General Failure Requirements

Malformed input MUST NOT be interpreted as a similar valid operation.

An unsupported operation MUST NOT cause the external action associated with
that operation. Invalid or ambiguous configuration MUST fail without causing
the Device to infer or invent intent.

A session boundary or reset MUST NOT cause an old operation, result, or
externally observable action to be replayed into a new session.

An extension that can cause an externally observable action MUST define the
configuration, activation, lifecycle, and other prerequisites for that action.
The Device MUST NOT perform the action unless those prerequisites hold.

Diagnostic state MUST NOT be used as control-authoritative state unless its
defining section explicitly permits that use.

Command-specific rejection, failure results, ordering, and recovery behavior
are defined by the applicable binding, profile, or extension.

## 10. Forward Compatibility

The meaning of an existing v1 field, operation, state, or result MUST NOT be
silently redefined. An incompatible wire change requires a new Device Protocol
version or an explicitly versioned extension whose rules permit that change.

Unknown enum values are not automatically equivalent to reserved bitmap bits.
A section may define reserved bitmap bits as ignorable to support compatible
expansion; that rule applies only to the identified bitmap.

An implementation MUST NOT claim support for a binding, compatibility profile,
or extension that it does not implement. A partial compatibility claim MUST
identify its supported profile or subset and MUST NOT change the meaning of the
supported parts.

Optional capabilities and extensions MUST NOT be inferred from implementation
hardware or from unspecified behavior. They must be declared as required by
their defining section.

## 11. Normative and Non-Normative Material

Requirements, definitions, wire semantics, and lifecycle rules in the main
sections of this specification are normative unless explicitly marked
otherwise.

Material explicitly introduced as **Note**, **Example**, **Implementation
note**, **Validation status**, or **Rationale** is non-normative. Such material
may explain an implementation or provide evidence, but it does not add a
conformance requirement.

Behavior of a reference implementation is not normative merely because it is
described as an example. Only behavior stated by the normative specification
defines protocol compatibility.

## 12. Protocol Bindings and Extensions

The [Pulse-Eight Serial Transport Binding](pulse8-serial-binding.md) is a
normative binding for Device Protocol v1. It defines how an ordered octet stream
is converted to decoded protocol frames without assigning meaning to those
frames.

The [Pulse-Eight Compatibility Profile](pulse8-compatibility-profile.md) is a
normative profile for implementations that claim it. It defines the supported
Pulse-Eight-compatible message subset carried by the serial binding, including
adapter state, CEC TX, asynchronous CEC RX, and follower acknowledgement.

[Wake Extension v1](wake-extension-v1.md) is the normative CEC Injector project
extension for transport-assisted host wake delivery. It allocates project
commands `0xE0` through `0xE7` and defines their state, lifecycle, trigger,
diagnostics, and conformance requirements.

[Device Protocol v1 Conformance](conformance.md) defines the meaning and
composition of Device and host compatibility claims. The
[Numeric Registry](numeric-registry.md) indexes frozen numeric values and links
each value to its normative definition.

Binding, profile, extension, conformance, and registry documents are subordinate
to this specification and are normative only within their declared scope. The
registry is an index and does not create semantics.
