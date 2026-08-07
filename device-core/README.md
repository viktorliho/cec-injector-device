# Device core

This area is reserved for platform-independent Device behavior. Its scope is
capabilities and delivery, including bounded mechanisms needed to deliver an
already-defined intent when host software cannot run.

User-facing host policy is outside this area and belongs to Mate.

The CMake target `CECInjector::DeviceCore` is a host-buildable static library.
The first migrated foundations define the common CEC frame limit and volatile
Pulse-Eight adapter state. Additional behavior will be added through controlled
migration.
