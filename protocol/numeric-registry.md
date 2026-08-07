# CEC Injector Device Protocol v1 Numeric Registry

**Part of CEC Injector Device Protocol v1**

**Maturity: Experimental**

## Status

This document is a normative index of numeric values defined elsewhere in CEC
Injector Device Protocol v1. It creates no semantics. If an entry and its linked
normative definition appear inconsistent, the defining document controls and
the registry MUST be corrected without changing that definition.

## 1. Pulse-Eight Serial Transport Binding

The [serial binding](pulse8-serial-binding.md) defines these values.

### 1.1 Framing and Escape Octets

| Value | Classification | Meaning | Definition |
|---:|---|---|---|
| `0xFA` | escape code | decoded `0xFD` | [Escaping](pulse8-serial-binding.md#6-escaping) |
| `0xFB` | escape code | decoded `0xFE` | [Escaping](pulse8-serial-binding.md#6-escaping) |
| `0xFC` | escape code | decoded `0xFF` | [Escaping](pulse8-serial-binding.md#6-escaping) |
| `0xFD` | framing octet | escape introducer | [Reserved framing octets](pulse8-serial-binding.md#4-reserved-framing-octets) |
| `0xFE` | framing octet | frame-end delimiter | [Reserved framing octets](pulse8-serial-binding.md#4-reserved-framing-octets) |
| `0xFF` | framing octet | frame-start and resynchronization delimiter | [Reserved framing octets](pulse8-serial-binding.md#4-reserved-framing-octets) |

| Limit | Value | Definition |
|---|---:|---|
| decoded frame length | 1–64 octets | [Decoded-frame limits](pulse8-serial-binding.md#7-decoded-frame-limits) |

The 64-octet limit belongs to this binding, not to every Device Protocol v1
binding.

## 2. Pulse-Eight Compatibility Profile

The [Pulse-Eight Compatibility Profile](pulse8-compatibility-profile.md)
defines the following decoded-message values.

### 2.1 Message Identifiers

| ID | Name | Classification | Definition |
|---:|---|---|---|
| `0x01` | `PING` | host request | [Basic operation](pulse8-compatibility-profile.md#51-ping-0x01) |
| `0x05` | `FRAME_START` | Device-to-host CEC RX base | [Asynchronous CEC RX](pulse8-compatibility-profile.md#12-asynchronous-cec-rx) |
| `0x06` | `FRAME_DATA` | Device-to-host CEC RX base | [Asynchronous CEC RX](pulse8-compatibility-profile.md#12-asynchronous-cec-rx) |
| `0x08` | `COMMAND_ACCEPTED` | generic Device response | [Acceptance and rejection](pulse8-compatibility-profile.md#31-generic-acceptance-and-rejection) |
| `0x09` | `COMMAND_REJECTED` | generic Device response | [Acceptance and rejection](pulse8-compatibility-profile.md#31-generic-acceptance-and-rejection) |
| `0x0A` | `SET_ACK_MASK` | host request | [ACK mask](pulse8-compatibility-profile.md#72-set_ack_mask-0x0a) |
| `0x0B` | `TRANSMIT` | host request | [TRANSMIT](pulse8-compatibility-profile.md#93-transmit-0x0b) |
| `0x0C` | `TRANSMIT_EOM` | host request | [TRANSMIT_EOM](pulse8-compatibility-profile.md#94-transmit_eom-0x0c) |
| `0x0D` | `TRANSMIT_IDLETIME` | host request | [TRANSMIT_IDLETIME](pulse8-compatibility-profile.md#91-transmit_idletime-0x0d) |
| `0x0E` | `TRANSMIT_ACK_POLARITY` | host request | [TRANSMIT_ACK_POLARITY](pulse8-compatibility-profile.md#92-transmit_ack_polarity-0x0e) |
| `0x10` | `TRANSMIT_SUCCEEDED` | asynchronous TX terminal | [TX terminal results](pulse8-compatibility-profile.md#11-tx-terminal-results) |
| `0x11` | `TRANSMIT_FAILED_LINE` | asynchronous TX terminal | [TX terminal results](pulse8-compatibility-profile.md#11-tx-terminal-results) |
| `0x12` | `TRANSMIT_FAILED_ACK` | asynchronous TX terminal | [TX terminal results](pulse8-compatibility-profile.md#11-tx-terminal-results) |
| `0x13` | `TRANSMIT_FAILED_TIMEOUT_DATA` | asynchronous TX terminal | [TX terminal results](pulse8-compatibility-profile.md#11-tx-terminal-results) |
| `0x14` | `TRANSMIT_FAILED_TIMEOUT_LINE` | asynchronous TX terminal | [TX terminal results](pulse8-compatibility-profile.md#11-tx-terminal-results) |
| `0x15` | `FIRMWARE_VERSION` | host request and getter response | [Compatibility metadata](pulse8-compatibility-profile.md#52-compatibility-metadata) |
| `0x17` | `GET_BUILDDATE` | host request and getter response | [Compatibility metadata](pulse8-compatibility-profile.md#52-compatibility-metadata) |
| `0x18` | `SET_CONTROLLED` | host request | [Controlled mode](pulse8-compatibility-profile.md#71-set_controlled-0x18) |
| `0x1B` | `GET_DEFAULT_LOGICAL_ADDRESS` | host request and getter response | [Default logical address](pulse8-compatibility-profile.md#73-default-logical-address-0x1b-0x1c) |
| `0x1C` | `SET_DEFAULT_LOGICAL_ADDRESS` | host request | [Default logical address](pulse8-compatibility-profile.md#73-default-logical-address-0x1b-0x1c) |
| `0x1D` | `GET_LOGICAL_ADDRESS_MASK` | host request and getter response | [Logical-address mask](pulse8-compatibility-profile.md#74-logical-address-mask-0x1d-0x1e) |
| `0x1E` | `SET_LOGICAL_ADDRESS_MASK` | host request | [Logical-address mask](pulse8-compatibility-profile.md#74-logical-address-mask-0x1d-0x1e) |
| `0x1F` | `GET_PHYSICAL_ADDRESS` | host request and getter response | [Physical address](pulse8-compatibility-profile.md#75-physical-address-0x1f-0x20) |
| `0x20` | `SET_PHYSICAL_ADDRESS` | host request | [Physical address](pulse8-compatibility-profile.md#75-physical-address-0x1f-0x20) |
| `0x28` | `GET_ADAPTER_TYPE` | host request and getter response | [Compatibility metadata](pulse8-compatibility-profile.md#52-compatibility-metadata) |

### 2.2 CEC RX Flag Bits

| Value | Name | Definition |
|---:|---|---|
| `0x40` | `ACK` | [Asynchronous CEC RX](pulse8-compatibility-profile.md#12-asynchronous-cec-rx) |
| `0x80` | `EOM` | [Asynchronous CEC RX](pulse8-compatibility-profile.md#12-asynchronous-cec-rx) |

The `ACK` name retains the profile's counterintuitive compatibility meaning:
on the header message, a set bit represents a physically high, non-acknowledged
ACK slot. The linked profile is authoritative.

### 2.3 Compatibility Values

| Value | Classification | Definition |
|---:|---|---|
| `0x0002` | firmware compatibility value | [Compatibility metadata](pulse8-compatibility-profile.md#52-compatibility-metadata) |
| `0x6A6E8880` | build-date compatibility value | [Compatibility metadata](pulse8-compatibility-profile.md#52-compatibility-metadata) |
| `0x01` | external adapter-type compatibility value | [Compatibility metadata](pulse8-compatibility-profile.md#52-compatibility-metadata) |

These are not CEC Injector firmware, build, protocol-version, or platform
metadata.

### 2.4 TX Constants

| Constant | Value | Definition |
|---|---:|---|
| maximum CEC TX frame length | 16 octets | [TX transaction model](pulse8-compatibility-profile.md#8-cec-tx-transaction-model) |
| partial assembly timeout | 1,000,000 microseconds | [TX transaction model](pulse8-compatibility-profile.md#8-cec-tx-transaction-model) |
| idletime compatibility default | `0x03` | [Adapter runtime state](pulse8-compatibility-profile.md#6-adapter-runtime-state) |

The idletime value does not normatively select physical bus-idle timing.

## 3. CEC Injector Project Command Namespace

Device Protocol v1 allocates exactly `0xE0` through `0xE7` to Wake Extension v1.
The [Wake Extension namespace](wake-extension-v1.md#4-cec-injector-project-command-namespace)
defines ownership. No value at or above `0xE8` is allocated by v1, and this
registry does not declare that space permanently free.

| ID | Command | Classification | Definition |
|---:|---|---|---|
| `0xE0` | `GET_WAKE_CAPABILITIES` | host request and getter response | [Capabilities](wake-extension-v1.md#7-capabilities--get_wake_capabilities-0xe0) |
| `0xE1` | `SET_WAKE_CONFIG` | host request | [Wake configuration](wake-extension-v1.md#82-set_wake_config-0xe1) |
| `0xE2` | `GET_WAKE_CONFIG` | host request and getter response | [Wake configuration](wake-extension-v1.md#83-get_wake_config-0xe2) |
| `0xE3` | `ARM_WAKE` | host request | [Arming](wake-extension-v1.md#91-arm_wake-0xe3) |
| `0xE4` | `DISARM_WAKE` | host request | [Disarming](wake-extension-v1.md#92-disarm_wake-0xe4) |
| `0xE5` | `GET_WAKE_STATUS` | host request and getter response | [Wake status](wake-extension-v1.md#10-wake-status--get_wake_status-0xe5) |
| `0xE6` | `GET_LAST_WAKE_REASON` | host request and getter response | [Last wake reason](wake-extension-v1.md#18-last-wake-reason--get_last_wake_reason-0xe6) |
| `0xE7` | `CLEAR_LAST_WAKE_REASON` | host request | [Clear last wake reason](wake-extension-v1.md#181-clear_last_wake_reason-0xe7) |

## 4. Wake Extension v1 Values

All values in this section are defined by
[Wake Extension v1](wake-extension-v1.md).

### 4.1 Version, Mode, and Capabilities

| Value | Meaning | Definition |
|---:|---|---|
| `0x01` | Wake Extension wire version 1 | [Versioning](wake-extension-v1.md#5-versioning) |
| `0x01` | mode 1, `SET_STREAM_PATH` | [Exact trigger](wake-extension-v1.md#14-exact-wake-trigger-and-eligibility) |
| `0x01` | modes bitmap: bit 0 advertises mode 1 | [Capabilities](wake-extension-v1.md#7-capabilities--get_wake_capabilities-0xe0) |
| `0x00` | capability flags: no optional flags | [Capabilities](wake-extension-v1.md#7-capabilities--get_wake_capabilities-0xe0) |

The repeated value `0x01` belongs to three distinct typed fields and MUST be
interpreted in field context.

### 4.2 E5 Status Flag Bits

| Value | Meaning | Definition |
|---:|---|---|
| `0x01` | configuration valid | [Status flags](wake-extension-v1.md#11-status-flags) |
| `0x02` | configuration enabled | [Status flags](wake-extension-v1.md#11-status-flags) |
| `0x04` | armed | [Status flags](wake-extension-v1.md#11-status-flags) |
| `0x08` | transport configured | [Status flags](wake-extension-v1.md#11-status-flags) |
| `0x10` | transport suspended | [Status flags](wake-extension-v1.md#11-status-flags) |
| `0x20` | host remote wake authorized | [Status flags](wake-extension-v1.md#11-status-flags) |
| `0x40` | wake request consumed in current suspend cycle | [Status flags](wake-extension-v1.md#11-status-flags) |
| `0x80` | reserved; Device emits zero | [Status flags](wake-extension-v1.md#11-status-flags) |

### 4.3 Derived State Values

| Value | Summary | Definition |
|---:|---|---|
| `0` | disabled or no valid enabled configuration | [Derived states](wake-extension-v1.md#12-derived-state-values) |
| `1` | valid enabled configuration, not armed | [Derived states](wake-extension-v1.md#12-derived-state-values) |
| `2` | armed, not currently eligible as state 3 | [Derived states](wake-extension-v1.md#12-derived-state-values) |
| `3` | armed, suspended, and authorized | [Derived states](wake-extension-v1.md#12-derived-state-values) |
| `4` | request consumed in current suspend cycle | [Derived states](wake-extension-v1.md#12-derived-state-values) |

### 4.4 Result Values

| Value | Name | Definition |
|---:|---|---|
| `0` | `NONE` | [Wake results](wake-extension-v1.md#13-wake-result-values) |
| `1` | `SUCCESS` | [Wake results](wake-extension-v1.md#13-wake-result-values) |
| `2` | `REMOTE_WAKE_FAILED` | [Wake results](wake-extension-v1.md#13-wake-result-values) |

### 4.5 Trigger and Delivery Constants

| Constant | Value | Definition |
|---|---:|---|
| CEC initiator | `0x0`, TV | [Exact trigger](wake-extension-v1.md#14-exact-wake-trigger-and-eligibility) |
| CEC destination | `0xF`, broadcast | [Exact trigger](wake-extension-v1.md#14-exact-wake-trigger-and-eligibility) |
| CEC opcode | `0x86`, `Set Stream Path` | [Exact trigger](wake-extension-v1.md#14-exact-wake-trigger-and-eligibility) |
| trigger frame length | 4 octets | [Exact trigger](wake-extension-v1.md#14-exact-wake-trigger-and-eligibility) |
| minimum delivery delay | 10,000 microseconds | [Wake delivery](wake-extension-v1.md#15-wake-delivery-and-duplicate-suppression) |
| maximum retained frame field | 16 octets | [Last wake reason](wake-extension-v1.md#18-last-wake-reason--get_last_wake_reason-0xe6) |

### 4.6 Diagnostic Integer Behavior

| Field class | Numeric behavior | Definition |
|---|---|---|
| E5 diagnostic counters | unsigned 32-bit, modulo 2^32 | [Diagnostic counters](wake-extension-v1.md#16-diagnostic-counters) |
| `last_wake_offset_us` | unsigned 32-bit, saturates at `0xFFFFFFFF` | [Last-wake offset](wake-extension-v1.md#17-last-wake-offset) |
