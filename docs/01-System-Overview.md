# System overview

## Current transitional architecture

```text
BLE devices
    -> ESP32 with SenseESP and sensesp-ble-gateway
    -> gateway protocol
    -> gateway provider
    -> current Signal K integration
    -> consumer plugins
    -> Signal K paths
```

## Target architecture

```text
BLE devices
    -> ESP32 with SenseESP and sensesp-ble-gateway
    -> gateway protocol
    -> gateway provider
    -> Signal K BLE Provider API
    -> consumer plugins
    -> Signal K paths
```

The gateway provider remains in both designs. Only its Signal K-facing adapter
changes when the official BLE Provider API becomes available. A local BlueZ
adapter may become another provider; it is not part of the ESP32 firmware.
