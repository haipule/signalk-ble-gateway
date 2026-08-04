# ADR-005: SenseESP as a firmware dependency

## Status

Accepted

## Decision

SenseESP v3 is a real dependency. BLE gateway functionality builds on
`dirkwa/sensesp-ble-gateway`.

## Consequences

- Networking, configuration, status, and OTA use SenseESP infrastructure.
- PlatformIO pins dependency revisions.
- Custom code is added only where existing libraries do not meet documented
  requirements.
- Material departures require a new architecture decision.
