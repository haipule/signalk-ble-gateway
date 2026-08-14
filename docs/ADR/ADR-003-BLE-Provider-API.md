# ADR-003: BLE Provider API as target integration

## Status

Accepted

## Context

Signal K PR #2588 has been merged and includes a built-in remote gateway
provider with HTTP advertisement ingestion and an optional GATT WebSocket.

## Decision

Firmware uses the official remote gateway protocol directly. Consumers use
`app.bleApi`. This project does not ship a parallel Signal K BLE provider.

## Consequences

- Signal K owns provider registration, device inventory and GATT claims.
- Consumers use the BLE Provider API rather than gateway transport details.
- The 0.1 implementation remains available as the rollback release line.
