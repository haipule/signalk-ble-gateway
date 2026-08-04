# Signal K BLE Provider API

The BLE Provider API proposed in RFC #2411 and PR #2588 is the target
integration point for the gateway provider, not a stable firmware interface.

The proposal includes:

- a unified advertisement stream,
- multiple concurrent BLE providers,
- server-side device inventory and deduplication,
- GATT claim management,
- provider selection by RSSI and availability,
- REST and WebSocket interfaces for consumers and applications.

## Migration boundary

The gateway provider encapsulates this dependency. Until the API ships, it
uses the project's transitional integration. Once available, only the
server-side adapter changes. HTTP POST and the future control WebSocket between
ESP32 and provider are private gateway protocol transports, not the public BLE
Provider API.
