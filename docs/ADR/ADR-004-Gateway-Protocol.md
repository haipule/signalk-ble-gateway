# ADR-004: Stable private gateway protocol

## Status

Superseded by the official remote gateway protocol merged in Signal K PR
#2588. Retained as the rationale for the 0.1 release line.

## Decision

The 0.1 firmware communicated through a private, versioned protocol. The 0.2
firmware posts to Signal K's official remote gateway endpoint; GATT remains a
later phase using the official WebSocket contract.

The design stays structurally compatible with `dirkwa/sensesp-ble-gateway`
where possible. Required extensions are documented and should be proposed
upstream when generally useful.

The examined upstream revision lacked protocol version, stable identity,
receive time, retained retry, and idempotency. It also coupled transport to the
primary SenseESP Signal K connection.

## Consequences

- Signal K internal APIs never become firmware dependencies.
- Transitional and target integrations use the same firmware.
- Model, authentication, limits, acknowledgement, and retry are specified
  before implementation.
