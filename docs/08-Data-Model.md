# Data model

The MVP model represents generic BLE advertisements without manufacturer
fields or Signal K paths.

## Required information

- protocol version and stable gateway ID,
- device address and address type,
- RSSI and gateway-relative receive timestamp,
- optional device name,
- complete advertising payload as hexadecimal bytes,
- optional complete scan-response payload.

Binary data is encoded unambiguously and losslessly. Multiple observations of
one device by different gateways remain separate transport events. Server-side
deduplication must retain original gateway and signal-quality information.

## Processing

```text
BLEAdvertisement -> gateway protocol -> provider -> consumer -> Signal K
```

Reference revision `ae35180d0d1f2cb6d043366a324c5b8320c001eb` of
`dirkwa/sensesp-ble-gateway` originally transmitted only `mac`, `rssi`,
optional `name`, and hexadecimal `adv_data`. Protocol v1 additionally preserves
`address_type`, `scan_rsp_data`, and `received_at_ms` from its internal model.
The complete raw payload remains authoritative; derived manufacturer or
service fields are optional server-side representations.
