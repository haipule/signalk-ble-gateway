# ESP32 gateway

The firmware uses SenseESP v3 and builds on
`dirkwa/sensesp-ble-gateway`. Departures from this foundation require an
architecture decision record.

## Advertisement MVP

The first gateway must reliably:

- capture passive BLE advertisements,
- preserve raw payloads with generic metadata,
- buffer observations in bounded queues,
- send versioned HTTP batches,
- recover from transient network and server failures,
- run without an attached USB monitor,
- expose remote status and diagnostics,
- support password-protected OTA updates.

## Architecture constraints

- The ESP32 performs transport and generic device interaction only.
- Firmware contains no manufacturer decoder or Signal K path.
- Manufacturer devices and decryption keys are never compiled into firmware.
- DHCP is the default; stable addressing is provided by a DHCP reservation.
- Generic metadata may include gateway ID, address type, RSSI, and timestamp.
- Advertising and scan-response payloads remain lossless raw byte sequences.

## Implemented work packages

1. PlatformIO project and reproducible dependency pins.
2. Wi-Fi, hostname, DHCP, authenticated web UI, and OTA foundation.
3. Headless heartbeat, reset, heap, BLE, queue, and delivery diagnostics.
4. Passive Bluedroid advertisement scanning.
5. Versioned generic advertisement model.
6. Bounded batching, retry, backoff, and queue overflow policy.
7. Authenticated HTTP delivery to the gateway provider.
8. Remote provider, path, failure, and endurance acceptance procedures.

GATT and its control WebSocket are outside the first MVP.
