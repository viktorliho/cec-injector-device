# GitHub Wiki Publication Map

**NON-NORMATIVE**

**Publication planning document**

## Authority and Purpose

Versioned documentation in this repository is canonical. The GitHub Wiki is an
explanatory and navigation layer for readers with different entry points and
levels of familiarity.

If Wiki material conflicts with a versioned repository specification, the
repository specification wins. Wiki pages should link to normative documents
instead of copying protocol tables or requirements. Updating a normative
contract requires a repository change and review; editing the Wiki cannot
change that contract.

This map proposes page scope only. It does not contain the future Wiki pages.

## Initial Page Map

| Page | Purpose | Audience | Canonical repository sources | Launch |
|---|---|---|---|---|
| Home | Provide a short project entry point and route readers to canonical material. | All readers | [`README.md`](../README.md), [`docs/design-charter.md`](design-charter.md), [`protocol/README.md`](../protocol/README.md) | LAUNCH |
| What is CEC Injector? | Explain the programmable HDMI-CEC endpoint and why the project exists. | New users and contributors | [`README.md`](../README.md), [`docs/design-charter.md`](design-charter.md) | LAUNCH |
| Architecture Overview | Summarize the Device, host, transport, extension, and delivery boundaries. | Implementers and reviewers | [`docs/design-charter.md`](design-charter.md), [`protocol/specification.md`](../protocol/specification.md) | LAUNCH |
| Device and Mate | Explain ownership of delivery and host policy without making Mate mandatory. | Host and Device developers | [`docs/design-charter.md`](design-charter.md), [`protocol/specification.md`](../protocol/specification.md) | LAUNCH |
| Protocol Overview | Navigate the normative v1 document set and explain composable claims. | Protocol implementers | [`protocol/README.md`](../protocol/README.md), [`protocol/specification.md`](../protocol/specification.md), [`protocol/conformance.md`](../protocol/conformance.md), [`protocol/numeric-registry.md`](../protocol/numeric-registry.md) | LAUNCH |
| Serial Framing | Explain where to find ordered-stream framing and examples without duplicating them. | Transport implementers | [`protocol/pulse8-serial-binding.md`](../protocol/pulse8-serial-binding.md) | LAUNCH |
| Pulse-Eight Compatibility | Explain the scope and limitations of the compatibility profile. | Device and host implementers | [`protocol/pulse8-compatibility-profile.md`](../protocol/pulse8-compatibility-profile.md), [`protocol/conformance.md`](../protocol/conformance.md) | LAUNCH |
| Wake Extension | Introduce discovery, configuration, trigger, delivery, and diagnostics and link to the contract. | Device and host implementers | [`protocol/wake-extension-v1.md`](../protocol/wake-extension-v1.md), [`protocol/conformance.md`](../protocol/conformance.md) | LAUNCH |
| Building a Device Implementation | Guide a new implementation from claims through conformance and platform integration. | Device implementers | [`protocol/specification.md`](../protocol/specification.md), [`protocol/conformance.md`](../protocol/conformance.md), future conformance tests and implementation guidance | LATER |
| Building a Host Implementation | Guide encoding, lifecycle handling, profile selection, and extension use. | Host implementers | [`protocol/specification.md`](../protocol/specification.md), [`protocol/conformance.md`](../protocol/conformance.md), future examples and fixtures | LATER |
| RP2040 Reference Platform | Describe the supported reference platform, build, and deployment after controlled migration. | Firmware developers and builders | [`platforms/rp2040/README.md`](../platforms/rp2040/README.md), future migrated platform sources | LATER |
| Hardware Prototype | Describe prototype electrical, PCB, and enclosure status with appropriate limitations. | Hardware developers and evaluators | [`hardware/README.md`](../hardware/README.md), future reviewed hardware sources | LATER |
| Compatibility and Tested Hardware | Separate normative compatibility claims from recorded validation environments. | Evaluators and integrators | [`protocol/conformance.md`](../protocol/conformance.md), protocol validation-status notes, future reviewed validation records | LATER |
| FAQ | Answer recurring scope, compatibility, maturity, safety, and licensing questions by linking to canonical sources. | All readers | [`README.md`](../README.md), [`docs/design-charter.md`](design-charter.md), [`protocol/README.md`](../protocol/README.md) | LAUNCH |

Launch plan: **9 LAUNCH pages** and **5 LATER pages**.
