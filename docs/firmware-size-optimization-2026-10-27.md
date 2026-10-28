# Firmware size optimization evidence — 2026-10-27

The standalone ESP32 firmware was rebuilt with ESP-IDF size optimization while
preserving assertions, features, partition layout, and dependency revisions.
No device was contacted or modified.

## Change

`CONFIG_COMPILER_OPTIMIZATION_SIZE=y` replaces the ESP-IDF default debug
optimization. Assertions remain enabled.

## Controlled build inputs

- PlatformIO environment: `az_delivery_devkit_v4_foundation`
- ESP32 platform: `55.03.37`
- ESP-IDF: `5.5.2`
- Arduino-ESP32: `3.3.7`
- SensESP: `d3e8a6fda56c5be866be7a21fe288a5e31144dfa`
- SensESP BLE Gateway: `ae35180d0d1f2cb6d043366a324c5b8320c001eb`
- `esp_websocket_client`: build `20260804121937`

The initially resolved newer WebSocket component was rejected for the
comparison. The successful build reused the exact component revision from the
reference build.

## Results

| Metric | Debug optimization | Size optimization | Improvement |
| --- | ---: | ---: | ---: |
| Application flash | 1,891,589 bytes | 1,719,131 bytes | 172,458 bytes |
| Free application partition | 74,491 bytes | 246,949 bytes | 172,458 bytes |
| IRAM | 121,223 bytes | 114,615 bytes | 6,608 bytes |
| Free IRAM | 9,849 bytes | 16,457 bytes | 6,608 bytes |
| DRAM | 62,288 bytes | 61,772 bytes | 516 bytes |
| Total image | 1,891,877 bytes | 1,719,419 bytes | 172,458 bytes |

The full clean build completed successfully in 52 minutes 53.737 seconds. A
warm incremental build completed successfully in 61.008 seconds and reproduced
the same resource values.

## Boundary

This evidence proves build and link compatibility only. It does not replace
the separately approved onboard validation required before OTA deployment.
