# Contributing

CEC Injector is Experimental. Before proposing architectural changes, read the
[Design Charter](docs/design-charter.md), especially its ownership and layer
boundaries.

Keep responsibilities separated:

- DeviceCore owns platform-neutral Device behavior;
- platform code owns hardware and transport binding; and
- Mate or other host software owns host policy and is outside this repository.

See the [build guide](docs/building.md) for complete prerequisites and firmware
build instructions. Run the host test suite before submitting a change:

```sh
cmake --preset host-debug
cmake --build --preset host-debug
ctest --preset host-debug
```

Add deterministic tests for new behavior where practical. Changes involving
electrical timing, USB lifecycle, or other hardware-sensitive behavior should
also include relevant hardware evidence; host tests alone do not establish
physical correctness.

The documents under [`protocol/`](protocol/README.md) define the normative wire
behavior. Do not change Protocol v1 semantics silently. Breaking wire changes
require explicit versioning rather than reinterpretation of an existing
version.

Preserve third-party copyright, attribution, and license terms described in
[`THIRD_PARTY.md`](THIRD_PARTY.md). Do not commit generated build artifacts.
