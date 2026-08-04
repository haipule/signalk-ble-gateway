# ADR-002: Manufacturer decoders are server-side consumers

## Status

Accepted

## Decision

All manufacturer protocols, device configuration, and keys reside in
server-side consumer plugins, including encrypted Victron advertisements.

## Consequences

- Firmware and provider transport raw data without interpretation.
- A MAC address may be generic observation data, but not compiled
  manufacturer configuration.
- Consumers publish Signal K paths.
- Keys remain on the server and are never distributed to gateways.
