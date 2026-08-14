# Consumer plugins

Consumer plugins contain domain and manufacturer logic. They:

- select relevant devices,
- manage manufacturer configuration and keys,
- decrypt and decode raw data,
- validate measurements,
- publish Signal K paths and deltas.

Consumers do not access the ESP32 or HTTP gateway protocol directly.

## Official consumer interface

Consumers subscribe to the merged provider stream through
`app.bleApi.onAdvertisement(pluginId, callback)`. Signal K supplies parsed
`manufacturerData`, `serviceData`, provider identity, RSSI, MAC, and timestamp.
The Victron consumer reads company ID `0x02E1` (decimal key `737`) from
`manufacturerData`; it does not parse the gateway HTTP body.

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
