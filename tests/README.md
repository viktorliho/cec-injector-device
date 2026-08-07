# Tests

This area contains Device-focused host tests and the public protocol
conformance-test boundary. Host tests compile the platform-neutral Device core
without requiring the Pico SDK or an Arm toolchain.

Run it from a clean build directory with:

```sh
cmake --preset host-debug
cmake --build --preset host-debug
ctest --preset host-debug
```

No private tests, captures, fixtures, or platform implementation have been
migrated.
