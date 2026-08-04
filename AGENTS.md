# AGENTS.md

# Signal K BLE Gateway

This repository develops a universal BLE gateway for Signal K. Long-term
maintainability depends on strict separation between firmware, gateway
protocol, Signal K integration, and manufacturer consumers.

## Architecture

```text
BLE devices -> ESP32 gateway -> gateway protocol
            -> Signal K adapter -> consumer plugins -> Signal K
```

### ESP32 firmware

Responsible for BLE scanning, generic GATT operations when implemented,
connection management, raw-data transport, bounded buffering, and retry.

Firmware must not contain manufacturer logic, Signal K paths, decryption keys,
or Signal K internal APIs.

### Gateway protocol

The protocol is the firmware's only public interface and should remain stable.

### Signal K adapter

The provider receives gateway messages and encapsulates current and future
Signal K integration, including migration to the official BLE Provider API.

### Consumer plugins

Consumers contain manufacturer decoding, interpretation, configuration,
business rules, and Signal K delta generation.

## Development rules

Before implementation:

1. Read `docs/02-Overall-Architecture.md`.
2. Follow relevant ADRs.
3. Preserve layer boundaries.
4. Write small, documented, testable modules.
5. Prefer asynchronous implementations.
6. Document public interfaces and architecture changes.
7. Keep commits focused and traceable.

Prefer modern C++, RAII, `const` correctness, strong typing, explicit error
handling, and mockable hardware interfaces. Avoid global mutable state, long
functions, duplicate logic, magic numbers, and unnecessary dependencies.

When alternatives exist, choose the design that keeps firmware generic,
minimizes coupling, and confines Signal K changes to the adapter.
