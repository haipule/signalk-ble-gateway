# ADR-001: Generic BLE transport on the ESP32

## Status

Accepted

## Decision

The ESP32 performs BLE capture, generic device interaction, and network
transport only. The advertisement MVP sends HTTP batches. GATT over a control
WebSocket is deferred. SenseESP v3 and `dirkwa/sensesp-ble-gateway` are the
firmware foundation.

## Consequences

- No manufacturer decoder or Signal K path in firmware.
- No manufacturer device allowlist or decryption key in firmware.
- The gateway protocol is stable and versioned.
- DHCP is standard; reservations provide stable addresses.
