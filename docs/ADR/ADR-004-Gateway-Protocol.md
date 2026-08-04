# ADR-004: Stable private gateway protocol

## Status

Accepted

## Decision

Firmware communicates only through a private, versioned protocol. The
advertisement MVP uses HTTP POST; a control WebSocket is specified only for a
later GATT phase.

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
