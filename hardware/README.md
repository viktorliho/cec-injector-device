# Experimental RP2040 Prototype Hardware

> **Experimental prototype only.** This wiring is not HDMI-certified,
> electrically safety-certified, or a production-ready reference design.
> Incorrect HDMI wiring may damage connected equipment. Reproduce and connect
> it at your own risk.

This document describes only the RP2040 electrical prototype that has actually
been developed and validated. RP2040 is currently the only supported reference
implementation for that reason; it is not an architectural requirement of CEC
Injector. See the model-specific [validation and compatibility
record](../docs/validation.md) for the exact scope of hardware evidence.

## Validated wiring

| HDMI connection | Prototype connection |
|---|---|
| HDMI pin 13 — CEC | 1 kΩ series resistor to RP2040 GP2 |
| HDMI pin 17 — ground | RP2040 GND |

```text
HDMI pin 13 (CEC) ---- 1 kΩ ---- RP2040 GP2

HDMI pin 17 (GND) -------------- RP2040 GND
```

GP2 is the firmware default. A different RP2040 GPIO can be selected at build
time with `CEC_INJECTOR_RP2040_CEC_GPIO`; changing the pin creates a different
hardware configuration from the one recorded here. See the [build
guide](../docs/building.md) and [RP2040 platform
documentation](../platforms/rp2040/README.md).

## CEC line invariant

The Device must never actively drive the CEC line HIGH.

The reference platform implements the two line states as:

```text
LOW  -> configure/drive the GPIO output LOW
HIGH -> release the GPIO to high impedance
```

The shared HDMI ground connection is required for the validated prototype. The
1 kΩ series resistor is part of that prototype, but its presence does not turn
this wiring description into a complete protected or production-qualified
electrical design.

## Scope and limitations

This prototype documentation does not provide or claim:

- a production schematic, PCB, or Gerber files;
- an ESD, surge, fault, or other protection design;
- EMC or signal-integrity validation;
- an enclosure or manufacturing BOM;
- HDMI or electrical-safety certification guidance; or
- universal compatibility with televisions or HDMI systems.

Those areas have not been fully evaluated by the project and must not be
inferred from successful operation of the validated prototype.
