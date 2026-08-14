# Signal K BLE Gateway

An open, universal ESP32 BLE advertisement gateway for Signal K.

The project deliberately separates transport, Signal K integration, and
manufacturer-specific decoding:

```text
BLE devices -> ESP32 gateway -> official remote gateway HTTP API
            -> Signal K BLE Provider API -> consumer plugins -> Signal K
```

The ESP32 forwards raw BLE advertisements. It does not contain Victron logic,
decryption keys, or Signal K paths. Manufacturer logic runs exclusively in
server-side consumer plugins.

## Current status: 0.2.0 migration branch

This branch is prepared for the official BLE Provider API merged by Signal K
PR #2588. The working legacy implementation remains available as
`v0.1.0-rc.1` and on the 0.1 release line.

- passive BLE scanning on an ESP32,
- bounded advertisement queues and HTTP batches with retry/backoff,
- authenticated batches to Signal K's built-in remote BLE provider,
- consumer subscription through `app.bleApi`,
- server-side Victron advertisement decryption,
- Lynx Smart BMS decoding,
- Orion XS decoding,
- Orion Signal K output paths verified on a live server,
- provider and consumer lifecycle separation verified,
- automated provider and consumer tests passing,
- ESP32 firmware build passing.

Lynx Smart BMS values are published below:

```text
electrical.batteries.<device-id>
```

Because the current Signal K schema has no dedicated DC/DC charger group,
Orion XS values are published as a charger:

```text
electrical.chargers.<device-id>.voltage
electrical.chargers.<device-id>.current
electrical.chargers.<device-id>.inputVoltage
electrical.chargers.<device-id>.inputCurrent
```

The remaining release gate is a long-duration hardware run with stable power.
A previously observed brownout was a supply reset, not a firmware stability
failure and not a successful endurance test.

GATT remains outside the advertisement MVP. The gateway WebSocket will be
implemented only when a consumer needs GATT.

## Components

- `firmware/`: generic ESP32 BLE advertisement gateway
- `consumer-plugin/`: Victron decoder and diagnostic web application
- `docs/`: architecture, protocol, operation, testing, and roadmap

## Quick start

1. Install a Signal K version containing BLE Provider API PR #2588.
2. Install and configure `consumer-plugin/` for the Victron devices.
3. Copy `firmware/secrets.example.h` to
   `firmware/include/secrets.h` and enter local credentials.
4. Build and flash the ESP32 from `firmware/`.

Until the BLE API reaches a stable Signal K release, use this branch only on a
test server. Keep the `v0.1.x` installation available for rollback.

Detailed installation and acceptance instructions are in
[`docs/15-Operations-and-Diagnostics.md`](docs/15-Operations-and-Diagnostics.md).
The release scope and remaining work are tracked in
[`docs/14-Roadmap.md`](docs/14-Roadmap.md).

## Public discussion

The repository is intentionally public to support architecture and protocol
discussion with the Signal K and marine electronics communities. Please open a
GitHub issue for reproducible defects or design proposals. Never post BLE
decryption keys, Signal K access tokens, Wi-Fi credentials, or vessel-specific
network details.

## License

Apache License 2.0. See [`LICENSE`](LICENSE).
