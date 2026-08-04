# Overall architecture

Transport, Signal K integration, and manufacturer logic are strictly separated.

## ESP32 firmware

Firmware captures advertisements, normalizes only generic BLE metadata, and
forwards raw payloads. A later version may execute generic GATT operations. It
does not know Signal K paths, manufacturer protocols, device keys, or business
rules.

## Gateway protocol

The gateway protocol is the firmware's only public interface. The MVP uses
batched HTTP POST requests for advertisements. A control WebSocket is reserved
for a later GATT phase. The protocol is versioned independently of Signal K
internals and aligned with `dirkwa/sensesp-ble-gateway` where practical.

## Gateway provider

The provider is a server-side Signal K plugin. It receives gateway messages,
tracks gateways, exposes diagnostics, and encapsulates Signal K integration.
Migration to the official BLE Provider API must not require firmware or
gateway-protocol changes.

## Consumer plugins

Consumers select devices, manage manufacturer configuration and keys, decrypt
and decode raw data, validate measurements, and publish Signal K deltas. Device
MAC addresses and advertisement keys belong in server-side consumer
configuration, never in firmware.

## Dependency rule

```text
Firmware -> gateway protocol <- gateway provider -> Signal K integration
                                             \----> BLE Provider API (target)

Consumer plugins -> provider consumer API -> Signal K
```

Consumer plugins never communicate directly with the ESP32 or HTTP transport.

## References

- [Signal K RFC #2411](https://github.com/SignalK/signalk-server/issues/2411)
- [Signal K PR #2588](https://github.com/SignalK/signalk-server/pull/2588)
- [dirkwa/sensesp-ble-gateway](https://github.com/dirkwa/sensesp-ble-gateway)
