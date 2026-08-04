# HTTP API

The ESP32 sends advertisement batches to the gateway provider by HTTP POST.
This is the private gateway protocol, not the public Signal K BLE Provider API.

## Protocol v1 endpoint

```http
POST /plugins/signalk-ble-gateway-provider/advertisements
Content-Type: application/json
Authorization: Bearer <gateway-provider-token>
```

Signal K currently protects plugin routes in this namespace as administrative
routes. The MVP therefore uses a dedicated gateway account. This broad
permission is transitional and must be replaced by a dedicated provider
credential or the official BLE Provider API. Tokens are never stored or logged
by the provider.

## Request

```json
{
  "protocol_version": "1",
  "gateway_id": "engine-room-stbd",
  "boot_id": "A1B2C3D4",
  "batch_id": "engine-room-stbd:A1B2C3D4:42",
  "sequence": 42,
  "uptime_ms": 123456,
  "firmware": "0.1.0-foundation",
  "devices": [
    {
      "mac": "11:22:33:44:55:66",
      "address_type": "random",
      "rssi": -67,
      "received_at_ms": 123400,
      "name": "optional",
      "adv_data": "020106...",
      "scan_rsp_data": "..."
    }
  ]
}
```

Required batch fields are `protocol_version`, `gateway_id`, `boot_id`,
`batch_id`, `sequence`, `uptime_ms`, and `devices`. Each observation requires
`mac`, `address_type`, `rssi`, `received_at_ms`, and `adv_data`.

- MAC addresses and hexadecimal strings use uppercase.
- `adv_data` is the complete lossless advertising payload.
- Empty `scan_rsp_data` and `name` fields are omitted.
- `received_at_ms` is monotonic time relative to the current boot; `boot_id`
  makes it unambiguous across resets.
- `address_type` is `public`, `random`, `public_identity`,
  `random_identity`, or `unknown`.
- A batch contains 1 to 100 observations.
- Advertising and scan-response payloads are limited to 255 bytes each.

## Responses and lifecycle

First acceptance returns HTTP 200:

```json
{
  "accepted": true,
  "duplicate": false,
  "status": 200,
  "batch_id": "engine-room-stbd:A1B2C3D4:42"
}
```

A repeated `batch_id` is not processed again but returns HTTP 200 with
`duplicate: true`. Invalid batches return HTTP 400 with an `errors` array.
Signal K returns HTTP 401 or 403 for authentication failures.

When the plugin is disabled, its registered route remains reachable but
returns HTTP 503 with `accepted: false`. It does not change provider counters
or emit consumer events. `GET /status` remains available and reports the
boolean `running` state.

## Delivery semantics and limits

- Firmware treats every 2xx response as acknowledgement.
- Timeout, connection failure, HTTP 408, 429, or 5xx retains the in-flight
  batch and retries with bounded exponential backoff.
- HTTP 400 discards the invalid batch and increments diagnostics.
- HTTP 401/403 pauses transport and triggers SenseESP reauthorization.
- The provider retains accepted batch IDs for at least 24 hours, bounded to
  4096 entries.
- Diagnostic storage is bounded to 1000 advertisements and is not durable.
- Delivery is at least once; provider processing is idempotent by `batch_id`.

## Upstream comparison

The comparison uses `dirkwa/sensesp-ble-gateway` commit
`ae35180d0d1f2cb6d043366a324c5b8320c001eb`. That revision posts to the
provisional `/signalk/v2/api/ble/gateway/advertisements` endpoint and omits
protocol version, stable identity separate from hostname, address type,
scan-response data, receive time, durable retry, and idempotency identifiers.

Those behaviors are reference input, not protocol v1. The decisions and
migration boundary are normative in
[ADR-006](ADR/ADR-006-Gateway-Protocol-v1.md).
