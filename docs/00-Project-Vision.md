# Project vision

This project provides a universal, distributed BLE infrastructure for Signal K.

ESP32 gateways capture BLE data and forward it through a stable, versioned
gateway protocol to a server-side provider. Firmware remains independent of
Signal K and manufacturer protocols.

Long term, the gateway provider should register with the official Signal K BLE
Provider API. A central BLE manager can then coordinate local and remote
providers, while consumer plugins interpret raw data and publish Signal K
paths. Changes in Signal K should affect the provider adapter, not firmware or
the gateway protocol.
