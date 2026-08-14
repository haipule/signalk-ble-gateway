# Signal K BLE Provider API

The BLE Provider API from PR #2588 is merged into Signal K Server `master`.
This project uses its built-in remote gateway provider directly.

The merged implementation includes:

- a unified advertisement stream,
- multiple concurrent BLE providers,
- server-side device inventory and deduplication,
- GATT claim management,
- provider selection by RSSI and availability,
- REST and WebSocket interfaces for consumers and applications.

## Integration boundary

Firmware posts batches to
`/signalk/v2/api/ble/gateway/advertisements`. Consumer plugins subscribe with
`app.bleApi.onAdvertisement(pluginId, callback)`. A future GATT implementation
uses `/signalk/v2/api/ble/gateway/ws` and the published AsyncAPI contract.

Consumers require `app.bleApi` explicitly at startup. They report a clear
plugin error when the installed Signal K version does not provide it; there is
no silent fallback to the removed private event bus.
