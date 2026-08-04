# ADR-003: BLE Provider API as target integration

## Status

Accepted

## Context

The Signal K BLE Provider API is still under development and is not a stable
firmware interface.

## Decision

A dedicated gateway provider currently adapts protocol v1 to Signal K. The
same provider will later integrate with the official BLE Provider API.

## Consequences

- The provider remains the server-side abstraction boundary.
- Migration changes only its Signal K-facing adapter.
- Firmware and gateway protocol remain unchanged.
- Target-state consumers use the BLE Provider API rather than gateways.
