# ADR-007: Reusable remote BLE gateway library

## Status

Accepted

## Context

The standalone firmware and both engine-room applications currently carry
copies of the remote BLE gateway transport. This makes fixes difficult to
apply consistently and obscures the boundary between generic BLE transport
and vessel-specific application code.

The repository also publishes the `signalk-victron-ble-consumer` npm package.
The C++ library therefore needs an identity and release namespace that cannot
be mistaken for either the consumer or the upstream
`dirkwa/sensesp-ble-gateway` abstraction.

## Decision

### Ownership and dependency direction

This repository is initially the single source of truth for the C++ package
`signalk-remote-ble-gateway`.

```text
dirkwa/sensesp-ble-gateway
             |
             v
signalk-remote-ble-gateway
             |
             +--> standalone firmware
             +--> port engine-room application
             +--> starboard engine-room application
```

Applications may depend on the gateway library. The gateway library must not
depend on an application, the npm consumer, vessel configuration, or
manufacturer decoding. Production applications pin an immutable gateway tag
or exact commit, never a moving branch.

### Library boundary

The library owns generic advertisement collection, MAC normalization and
optional allowlist filtering, bounded queues, batching, retry/backoff, HTTP
delivery, scan recovery, and gateway diagnostics.

The application owns `setup()` and `loop()`, Wi-Fi and OTA setup, secrets,
local sensors and GPIO, Signal K sensor paths, web presentation, and global
Bluetooth-controller initialization or reset. In particular, the library must
not silently disable or deinitialize Bluedroid or the ESP32 Bluetooth
controller.

Manufacturer protocols, keys, interpretation, business rules, and Signal K
delta generation remain in consumer plugins.

### Small public interface

The first extracted interface consists of:

- a value-type configuration containing gateway identity, optional allowlist,
  delivery intervals, and bounded queue/batch sizes;
- construction with explicit BLE and Signal K client dependencies;
- idempotent `start()` and `stop()` lifecycle operations; and
- one read-only diagnostics snapshot containing running/scanning state,
  receive/delivery/filter/drop counters, queue depth, scan-start counters,
  HTTP status and duration, retry interval, and token availability.

The snapshot replaces growth through individual public counter getters. It is
observational only: reading diagnostics must not change transport state. The
current getters may remain temporarily as compatibility shims during the
behavior-preserving migration.

### Packaging and versions

The PlatformIO manifest is at the repository root so a Git URL can be used
directly in `lib_deps`. Its build paths point only at `gateway-library/include`
and `gateway-library/src`; standalone firmware and npm consumer sources are
not part of the C++ build.

Release tags use separate namespaces:

- `vX.Y.Z` remains the npm consumer release namespace and is the only pattern
  accepted by the npm publish workflow;
- `gateway-vX.Y.Z` identifies immutable C++ gateway-library releases and must
  not trigger npm publication.

The manifest version and the gateway tag version must agree for a library
release. The initial extraction remains unreleased until standalone and both
engine-room builds pass and approved hardware validation is complete.

## Consequences

- The existing applications continue unchanged during the packaging spike.
- The standalone firmware is migrated first and proves the local library
  boundary before either engine-room repository is edited.
- A later repository split remains possible without changing application
  includes or the package name, but is not justified until this layout fails a
  concrete maintenance or tooling requirement.
- Any public protocol or ownership change requires a new ADR.

