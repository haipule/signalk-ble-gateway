# Data model

The MVP model represents generic BLE advertisements without manufacturer
fields or Signal K paths.

## Required information

- stable gateway ID,
- device address and optional public/random address type,
- RSSI,
- optional device name,
- complete advertising payload as hexadecimal bytes,
- optional complete scan-response payload.

Binary data is encoded unambiguously and losslessly. Multiple observations of
one device by different gateways remain separate transport events. Server-side
deduplication must retain original gateway and signal-quality information.

## Processing

```text
BLEAdvertisement -> official remote provider -> BLE API -> consumer -> Signal K
```

The wire format transmits `mac`, `rssi`, optional `name`, hexadecimal
`adv_data`, optional `scan_rsp_data`, and optional `address_type`. Signal K
timestamps receipt and derives manufacturer and service fields server-side.
The complete raw payload remains authoritative on the gateway boundary.
