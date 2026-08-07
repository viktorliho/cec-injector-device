# Building CEC Injector Device Firmware

This guide covers two independent workflows:

- building and running the host-side automated tests; and
- cross-building the RP2040 reference firmware.

The repository does not vendor or automatically download a compiler, the
Raspberry Pi Pico SDK, TinyUSB, or an Arm toolchain. Keep dependency checkouts
and generated build directories outside the source tree where practical.

## Host tests

### Requirements

- CMake 3.25 or newer;
- a native C compiler with C11 support; and
- Make, as used by the current `host-debug` preset, or a compatible generator
  for an equivalent manual configuration.

From the repository root, run:

```sh
cmake --preset host-debug
cmake --build --preset host-debug
ctest --preset host-debug
```

This builds DeviceCore on the host and runs the adapter-state, protocol
conformance, and RP2040 platform-boundary tests. These deterministic tests do
not validate electrical timing, a physical USB controller, HDMI wiring, or
compatibility with a particular television or host environment.

## RP2040 firmware

### Validated dependencies

The hardware-validated firmware build used:

| Dependency | Validated version |
|---|---|
| Raspberry Pi Pico SDK | tag `2.2.0`, commit `a1438dff1d38bd9c65dbd693f0e5db4b9ae91779` |
| TinyUSB from the Pico SDK checkout | commit `86ad6e56c1700e85f1c5678607a762cfe3aa2f47` |
| Arm GNU Toolchain | `14.2.Rel1`; GCC `14.2.1 20241119` |
| CMake | 3.25 or newer |

These versions record the environment used for hardware validation. Except for
the explicit CMake minimum, they are not promises that every future source
revision will require the same versions.

Obtain the Pico SDK as an external checkout and initialize its submodules. For
example:

```sh
git clone --branch 2.2.0 --recurse-submodules \
  https://github.com/raspberrypi/pico-sdk.git /path/to/pico-sdk
git -C /path/to/pico-sdk checkout \
  a1438dff1d38bd9c65dbd693f0e5db4b9ae91779
git -C /path/to/pico-sdk submodule update --init --recursive
```

TinyUSB is supplied by that initialized SDK checkout; do not provide a second
TinyUSB tree to this project. Install the Arm GNU Toolchain separately and make
its `arm-none-eabi-*` programs available on `PATH`.

Set `PICO_SDK_PATH` to the external SDK checkout before configuring:

```sh
export PICO_SDK_PATH=/path/to/pico-sdk
```

### Debug build

```sh
cmake -S . -B build/rp2040-debug \
  -DCEC_INJECTOR_BUILD_TESTS=OFF \
  -DCEC_INJECTOR_BUILD_RP2040=ON \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build build/rp2040-debug --target cec_adapter
```

### Release build

```sh
cmake -S . -B build/rp2040-release \
  -DCEC_INJECTOR_BUILD_TESTS=OFF \
  -DCEC_INJECTOR_BUILD_RP2040=ON \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build/rp2040-release --target cec_adapter
```

`Release` is the normal distribution/reference firmware. `Debug` is intended
for development and is not the default user artifact. Neither build enables a
textual debug protocol or USB/UART console.

The production target is `cec_adapter`. With the build directories above, Pico
SDK extra outputs are normally found under:

```text
build/rp2040-{debug,release}/platforms/rp2040/
```

They include `cec_adapter.elf`, `cec_adapter.uf2`, `cec_adapter.bin`,
`cec_adapter.hex`, `cec_adapter.dis`, and `cec_adapter.elf.map` when supported
by the selected SDK/toolchain configuration.

## Reference-platform configuration

The following CMake cache settings are implemented by the current RP2040
build:

| Setting | Default | Purpose |
|---|---|---|
| `CEC_INJECTOR_RP2040_CEC_GPIO` | `2` | RP2040 GPIO used for the open-drain CEC line |
| `CEC_INJECTOR_USB_VID` | `0xCAFE` | Development USB vendor ID |
| `CEC_INJECTOR_USB_PID` | `0xCEC1` | Development USB product ID |
| `CEC_INJECTOR_USB_MANUFACTURER` | `CEC Injector Project` | USB manufacturer string |
| `CEC_INJECTOR_USB_PRODUCT` | `RP2040 HDMI-CEC Adapter (Development)` | USB product string |

Pass an override during CMake configuration, for example:

```sh
cmake -S . -B build/rp2040-release \
  -DCEC_INJECTOR_BUILD_TESTS=OFF \
  -DCEC_INJECTOR_BUILD_RP2040=ON \
  -DCMAKE_BUILD_TYPE=Release \
  -DCEC_INJECTOR_RP2040_CEC_GPIO=2 \
  -DCEC_INJECTOR_USB_VID=0xCAFE \
  -DCEC_INJECTOR_USB_PID=0xCEC1
```

`CAFE:CEC1` is a development/reference identity. It is not a globally assigned
CEC Injector VID/PID, is not Device Protocol identity, and must not be assumed
appropriate for commercial distribution. Builders and distributors are
responsible for using a legitimately assigned USB identity where required.

The production firmware owns one CDC byte stream for the Device protocol.
Pico USB stdio and UART stdio are disabled so console text cannot consume or
inject protocol bytes.

## Reproducibility and generated output

Binary hashes can depend on the exact source commit, SDK and submodule commits,
compiler/toolchain, build type, CMake settings, generator, and paths embedded
by development builds. Record those inputs when comparing artifacts.

The exact identity of a published or hardware-validated artifact belongs in
the applicable release or validation record, not in this general build guide.
Generated build products are ignored by Git and are not tracked in the source
repository.
