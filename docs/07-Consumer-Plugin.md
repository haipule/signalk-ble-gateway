# Consumer plugins

Consumer plugins contain domain and manufacturer logic. They:

- select relevant devices,
- manage manufacturer configuration and keys,
- decrypt and decode raw data,
- validate measurements,
- publish Signal K paths and deltas.

Consumers do not access the ESP32 or HTTP gateway protocol directly.

## Transitional consumer interface

Until the official API is available, the gateway provider emits every newly
accepted advertisement exactly once as internal event
`signalk-ble-gateway:advertisement:v1`. The event contains only the generic
provider model. Consumers subscribe through a small adapter, so decoders know
nothing about HTTP batches or routes.

Only this adapter should change during migration to the official BLE Provider
API.

## Victron consumer

`consumer-plugin/` stores MAC addresses and advertisement keys in Signal K
plugin configuration, selects company ID `0x02E1`, and decrypts records with
AES-128-CTR. Status responses expose only whether a key exists.

Lynx Smart BMS measurements are published below
`electrical.batteries.<device-id>`. Orion XS is represented below
`electrical.chargers.<device-id>` because the current schema lacks a dedicated
DC/DC charger group. Standard output paths are `voltage` and `current`; clearly
named extension paths `inputVoltage` and `inputCurrent` represent the source
side. State, error code, and shutdown reason remain diagnostic-only.
