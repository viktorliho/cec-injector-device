# CEC Injector Design Charter

> This document explains why CEC Injector is built the way it is.

## Document Status

The Design Charter defines the project's long-term goals, component boundaries, and principles for architectural decisions.

It is not a protocol specification, user guide, release roadmap, or description of the current implementation. It should not depend on a particular microcontroller, operating system, transport, build system, or repository layout.

The Charter may evolve when practical experience reveals a flaw in its principles or when the scope of the project changes substantially. Such changes should represent deliberate architectural decisions rather than temporary implementation convenience.

## 1. Mission

**CEC Injector is an open specification and a family of implementations for a programmable HDMI-CEC endpoint.**

The project exists to bring the convenience and integration of consumer HDMI-CEC devices to open computing platforms while preserving their flexibility, programmability, and user control.

CEC Injector defines an open contract between the Device and host software. This contract allows hardware platforms, host clients, and new use cases to evolve independently.

The project does not aim to reproduce one commercial product. It aims to provide an open foundation for compatible implementations and capabilities that closed architectures cannot offer.

## 2. Vision

Open computing platforms should not be less convenient than consumer electronics when interacting with a home AV environment.

Users should not have to choose between the comfort of a closed appliance and the freedom of an open platform.

CEC Injector aims to make HDMI-CEC a first-class part of open systems through a documented protocol, portable implementations, and a programmable architecture.

> **Consumer-grade UX without giving up the flexibility of an open platform.**

Advanced configuration should extend a good default experience, not be required for normal operation.

## 3. Core Architecture

### 3.1. Protocol before implementation

The public protocol is the primary contract of CEC Injector. It describes the observable behavior of a compatible Device independently of specific hardware, runtime, transport implementation, or source layout.

Implementations follow the protocol. Behavior found in one implementation does not become part of the protocol automatically.

> **The protocol describes the device.**  
> **The implementation does not define the protocol.**

While the protocol remains Experimental, preserving it must not permanently lock the project into a fundamental architectural mistake.

### 3.2. Device and Mate have different responsibilities

> **CEC Injector Device owns delivery.**  
> **CEC Injector Mate owns host policy.**

The fuller model is:

> **Capabilities belong to the Device.**  
> **Intent originates on the host.**  
> **Delivery belongs to the Device.**  
> **Host policy belongs to the Mate.**

The Device provides physical capabilities and bounded mechanisms for reliable delivery.

The host produces intent from user actions, operating-system state, application context, or other external policy.

The Mate interprets that intent and manages host behavior.

The Device must not invent user intent.

### 3.3. Reliable delivery without policy ownership

The Device may perform work needed to deliver an explicitly defined intent between the HDMI-CEC environment and host-facing mechanisms.

This may include translation between physical mechanisms, state confirmation, bounded retries, duplicate suppression, selection among pre-authorized delivery backends, and operation while host software is unavailable.

> **The Device may improve delivery. It must never invent intent.**

Retries, fallbacks, and autonomous actions must be bounded, deterministic, diagnosable, safe to repeat, and guaranteed to reach a terminal state.

Improving delivery reliability must not increase the authority of the Device.

### 3.4. Mechanism and policy remain separate

The Device provides mechanisms and capabilities. The host decides when and why to use them.

A decision belongs to host policy by default when it depends on user preferences, operating-system state, a particular application, deployment environment, or user session.

When an action must occur while host software cannot run, the Device may implement the required mechanism. Its configuration and activation should remain under host control whenever practical.

### 3.5. Minimal authority

The Device should have only the authority required by its declared capabilities and delivery contract.

Technical possibility alone is not sufficient reason to add a feature.

The Device must not become a general-purpose application platform merely because its hardware has spare processing power, networking, or storage.

## 4. Design Principles

### 4.1. Explicit contracts over hidden heuristics

Explicit configuration, state machines, capability discovery, and versioned messages are preferred over hidden assumptions.

When the Device cannot determine intent safely, it should reject the action or require explicit configuration rather than guess.

Malformed input must not be interpreted as a similar valid command.

### 4.2. Every feature has one owning layer

Every feature must have one clearly identified layer that owns its meaning and state.

Responsibility must not be duplicated implicitly across the protocol, device core, platform implementation, transport binding, Mate, and operating-system integration.

### 4.3. Do no harm

CEC Injector must not make the basic behavior of connected devices and links worse.

On malformed input, lost configuration, disconnect, failure, or ambiguous state, the Device should return to a safe passive state.

Without confirmed contract state, it must not unexpectedly transmit on the CEC bus, hold the CEC line, wake the host, activate a power output, replay an old operation after reconnect, or cause another externally observable action.

Failure or absence of CEC Injector should, whenever practical, leave the connected system no less usable than it would be without the Device.

A feature is unacceptable when its failure can create a more serious problem than the feature was intended to solve.

### 4.4. Consumer-grade UX is a core requirement

Consumer-grade UX is a primary engineering goal, not optional polish on top of a working prototype.

The minimum supported configuration should aim for plug-and-play behavior. Daily use should be reliable, predictable, timely, and free from regular manual maintenance.

Advanced configuration should extend the default experience rather than be required to make it usable.

An open system is not complete merely because an expert can make it work manually.

### 4.5. Portability without artificial abstraction

Portable logic should be separated from platform-specific code where a real shared contract exists.

The project should not create universal abstractions merely to appear universal.

Timing-critical electrical behavior, GPIO, interrupts, USB power management, and other hardware-sensitive mechanisms may remain platform-specific when abstraction would hide important properties, reduce testability, or weaken safety guarantees.

### 4.6. Validation is part of implementation

A feature is not complete merely because the code builds or passes unit tests.

Protocol behavior should be verified with deterministic tests. Electrical and externally observable behavior should be validated on real hardware where applicable.

The project distinguishes among Implemented, Tested, Hardware validated, and Supported.

### 4.7. Honest limitations over optimistic claims

The project must clearly distinguish specification, implementation, experimental prototype, reference design, supported platform, and production-ready design.

An unvalidated platform, TV model, or electrical frontend must not be claimed as compatible or safe without sufficient evidence.

## 5. Public Contract

The canonical protocol specification is the normative source for communication between the Device and host software.

It must contain enough information to build an independent compatible implementation without examining official source code, observing the official client, reverse-engineering a finished product, or accessing private development history.

The public contract covers observable behavior required for interoperability: commands, payloads, asynchronous messages, transport framing, error semantics, ordering, state transitions, capability discovery, defaults, versions, lifecycle, relevant timing, and malformed or unsupported input behavior.

Internal C or C++ interfaces, headers, data structures, module boundaries, directory layout, concurrency model, memory layout, debug CLI, diagnostic formats, and the build system are not stable public APIs unless declared otherwise.

The specification must distinguish required behavior, optional capabilities, versioned extensions, and implementation-specific behavior.

Extensions add capabilities without redefining existing base semantics.

The Device protocol and transport bindings are distinct. The protocol defines message meaning and the general lifecycle model. A transport binding defines framing, escaping, delivery assumptions, connection lifecycle, and medium-specific limits.

Connect, disconnect, reset, transport loss, suspend, and resume are part of the contract when they affect state or observable behavior.

State ownership and lifetime must be explicit.

Public knowledge must not depend on private development diaries, agent prompts, personal logs, or other closed materials.

## 6. Compatibility and Versioning

Compatibility is defined by behavior, not by source-code origin.

An implementation is compatible with a protocol version when it preserves the meaning of its declared contract, implements the required behavior of its declared subset, reports missing capabilities honestly, and passes applicable conformance tests.

Partial compatibility is valid when the implementation identifies the protocol version and clearly lists its implemented subset.

Passing conformance tests is necessary but not sufficient. It does not replace specification review, lifecycle testing, failure-path analysis, electrical validation, or real-hardware testing.

Every published protocol version is an independent historical specification. An incompatible change creates a new protocol version.

**Experimental** means incompatible changes remain possible.

**Stable** means wire semantics do not change incompatibly within the version.

**Legacy** means the version is no longer the main development target, while its specification and support status remain available.

Stability must be earned through specification completeness, independent conformance tests, absence of fundamental architectural blockers, and experience with multiple host implementations, Device platforms, and CEC environments.

## 7. Device Philosophy

The Device is a programmable HDMI-CEC endpoint, not merely a passive interface converter.

It may maintain state, capabilities, configuration, bounded autonomous mechanisms, and multiple delivery backends. Programmability does not give the Device authority to define user-facing policy.

The main role of the Device is to carry explicitly defined intent between the HDMI-CEC environment and host-facing mechanisms without changing the meaning of the action.

Dangerous or externally observable capabilities should have discovery, explicit configuration, activation semantics, safe defaults, and diagnosable state.

Autonomous behavior is acceptable only to carry out previously defined intent or maintain the protocol contract. It should have an unambiguous trigger, bounded actions, an overall deadline, duplicate suppression, a terminal state, and safe reset behavior.

After reset, physical disconnect, or loss of valid configuration, the Device should return to a defined safe state.

Additional capabilities extend the base role of CEC Injector but must not make the base contract depend on them.

## 8. Platform Philosophy

A platform implementation realizes CEC Injector on a particular combination of hardware, runtime, and available transports.

No platform defines the identity of the whole project or receives permanent architectural priority.

Constraints of one platform should not become constraints of the protocol without need.

Compatibility is determined by public behavior and the declared subset.

Logic belongs in the device core only when a real shared semantic contract exists.

Platform differences should be expressed through documented capabilities, versions, configuration, and lifecycle mappings.

Failure of an optional subsystem should not destroy an independent base transport without need.

## 9. Hardware Philosophy

Electrical implementation is part of the real device contract, not a secondary firmware detail.

Correct software does not compensate for a design that permits invalid voltage levels, a held bus line, unsafe power action, uncontrolled reset behavior, or damage to connected equipment.

Experimental wiring and frontends must be labeled Experimental next to reproduction instructions.

A working prototype does not imply production suitability, universal safety, formal HDMI compliance, certification, or guaranteed long-term reliability.

The electrical architecture should return to a safe passive state on reset, loss of power, or control-logic failure whenever practical.

For physically consequential actions, hardware should limit the consequences of software failure independently of firmware where practical.

An open hardware release should include editable design sources. Manufacturing outputs, renders, and photographs do not replace the source design.

Validation results should be tied to a specific hardware revision.

## 10. Device Repository Scope

The Device repository contains the material needed to understand, implement, build, and validate CEC Injector Device.

Its scope includes the canonical protocol specification, transport bindings, Device state and capabilities, portable logic, platform implementations, board configuration, conformance tests and fixtures, developer tools for building, flashing, protocol inspection, and validation, electrical documentation, hardware-validation procedures, future PCB and enclosure source designs, compatibility records, and scoped validation reports.

Its scope does not include a production host client, user-facing host service, operating-system integration, desktop or gaming-environment policy, CEC-to-keyboard or CEC-to-gamepad policy, host power-lifecycle implementation, a GUI for host behavior, application-specific integration, private development diaries, agent prompts, or personal logs.

Host-side code is acceptable only as Device development tooling. It must not become a hidden production client or a second source of protocol semantics.

The Device repository owns transport, capabilities, and bounded delivery mechanisms.

The Mate repository owns production host policy.

## 11. Open Source Philosophy

Openness is an engineering decision for CEC Injector.

The project's primary long-term asset is its fully published and maintained protocol.

> **Open source makes implementations reusable.**  
> **An open protocol makes the ecosystem independent.**

An independent developer should be able to build a Device or host implementation without reverse-engineering an official product.

Official implementations remain open source, but a third-party compatible implementation does not have to use the same code, internal architecture, or licensing model.

Forks, alternative implementations, and specialized derivatives are natural parts of the ecosystem.

The shared protocol should bind the ecosystem more strongly than a single codebase.

No official implementation, repository, or host application should become a mandatory point of existence for the whole system.

## 12. Evolution

A roadmap describes directions, not unconditional promises about dates or features.

A Device-side feature is accepted when it adds useful capability, does not increase Device authority, preserves the boundary between delivery and host policy, has bounded, deterministic, and diagnosable behavior, fails safely, does not make the system worse on failure, and is accompanied by public documentation and tests.

New platforms, transports, and capabilities should appear through explicit contracts, capability discovery, safe defaults, versioning, defined lifecycle behavior, conformance tests, and precise validation claims.

The project should not grow through undocumented heuristics and vendor-specific exceptions.

While a contract is Experimental, correcting a fundamental architectural error may take priority over preserving the old version. Such a change must create a new protocol version.

CEC Injector Device, CEC Injector Mate, and future ecosystem components may evolve independently.

A feature should live where its architectural responsibility belongs, not merely where it is easiest to implement.

## 13. Closing Principle

CEC Injector aims to combine consumer-grade UX with the independence and flexibility of open platforms.

The Device owns capabilities and delivery.

The host produces intent.

The Mate owns host policy.

An open protocol allows every component to be implemented independently and prevents the ecosystem from depending on a single vendor application.

The Device may make delivery more reliable, but it must not invent user intent.

New capabilities should improve the system without unnecessarily expanding Device authority or making basic behavior worse when they fail.

> **CEC Injector Device owns delivery.**  
> **CEC Injector Mate owns host policy.**
