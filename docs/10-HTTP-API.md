# Official remote gateway HTTP API

The ESP32 posts advertisement batches directly to Signal K's built-in remote
BLE provider:

```http
POST /signalk/v2/api/ble/gateway/advertisements
Content-Type: application/json
Authorization: Bearer <token>
```

The canonical contract is `BLEGatewayAdvertisementBatchSchema` in Signal K's
`packages/server-api/src/typebox/ble-schemas.ts`. The implementation was
prepared against Signal K commit
`2495b8600c0b3af43c22d6e54cd644ecbb3e4b05`.

```json
{
  "gateway_id": "engine-room-stbd",
  "firmware": "0.2.0",
  "mac": "AA:BB:CC:DD:EE:FF",
  "hostname": "engine-room-stbd",
  "uptime": 123,
  "free_heap": 123456,
  "devices": [
    {
      "mac": "11:22:33:44:55:66",
      "address_type": "random",
      "rssi": -67,
      "name": "optional",
      "adv_data": "020106...",
      "scan_rsp_data": "..."
    }
  ]
}
```

Only `public` and `random` are valid address types. When the scanner reports a
type that cannot be represented by the official schema, firmware omits the
field. Raw advertisement and scan-response bytes remain authoritative.

The firmware retains one in-flight batch and retries transient transport or
server failures with bounded exponential backoff. The official endpoint does
not expose the old private `batch_id` deduplication contract, so consumers must
tolerate repeated observations. Victron updates are naturally idempotent at
the Signal K path level.
