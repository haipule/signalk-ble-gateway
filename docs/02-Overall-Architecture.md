# Overall architecture

Transport, Signal K integration, and manufacturer logic are strictly separated.

## ESP32 firmware

Firmware captures advertisements, normalizes only generic BLE metadata, and
forwards raw payloads. A later version may execute generic GATT operations. It
does not know Signal K paths, manufacturer protocols, device keys, or business
rules.

## Remote gateway protocol

The firmware uses the official Signal K remote BLE gateway protocol. The MVP
posts advertisement batches directly to Signal K. The official gateway
WebSocket is reserved for a later GATT phase.

## Signal K BLE infrastructure

Signal K's built-in remote provider receives gateway messages, parses raw AD
structures, registers each gateway as a BLE provider, and exposes the merged
BLE API to consumers. This repository no longer ships a parallel provider.

## Consumer plugins

Consumers select devices, manage manufacturer configuration and keys, decrypt
and decode raw data, validate measurements, and publish Signal K deltas. Device
MAC addresses and advertisement keys belong in server-side consumer
configuration, never in firmware.

## Dependency rule

```text
Firmware -> official remote gateway protocol -> Signal K BLE Provider API
                                                     |
Consumer plugins <-----------------------------------+--> Signal K
```

Consumer plugins never communicate directly with the ESP32 or HTTP transport.

## References

- [Signal K RFC #2411](https://github.com/SignalK/signalk-server/issues/2411)
- [Signal K PR #2588](https://github.com/SignalK/signalk-server/pull/2588)
- [dirkwa/sensesp-ble-gateway](https://github.com/dirkwa/sensesp-ble-gateway)
