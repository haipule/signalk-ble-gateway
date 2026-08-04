# Signal K BLE Gateway Provider

Transitional Signal K provider for this project's versioned gateway protocol.
It accepts manufacturer-neutral BLE advertisement batches from remote gateways
and exposes bounded in-memory diagnostics.

The provider is intentionally isolated from the proposed Signal K BLE Provider
API. Once that API is released, only the server-side adapter should change;
firmware and gateway protocol remain stable.

## Development installation

On the Signal K host:

```sh
cd /path/to/signalk-ble-gateway/plugin
npm link
cd ~/.signalk
npm link signalk-ble-gateway-provider
```

Restart Signal K and enable the plugin under **Server -> Plugin Config**.

## HTTP endpoints

All routes are below:

```text
/plugins/signalk-ble-gateway-provider
```

- `POST /advertisements`: accept a protocol-v1 batch
- `GET /status`: provider and gateway diagnostics
- `GET /advertisements`: bounded stream of recently received advertisements

Signal K access control protects these endpoints. The ESP sends a read-write
token as `Authorization: Bearer ...`.

The normative payload and response format is documented in
[`../docs/10-HTTP-API.md`](../docs/10-HTTP-API.md).

## Tests

```sh
npm test
```
