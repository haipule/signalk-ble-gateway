# ESP32 firmware

The firmware passively scans BLE advertisements and sends the unmodified raw
data to the Signal K gateway provider in bounded batches. Manufacturer
decoding and Signal K paths remain outside the firmware.

The remote BLE gateway endpoint requires Signal K Server 2.31 or newer.

## Target board

The target is the **AZ-Delivery ESP-32 Dev Kit C V4**, using PlatformIO board
ID `az-delivery-devkit-v4` and environment
`az_delivery_devkit_v4_foundation`.

## Local configuration

1. Copy `secrets.example.h` to `include/secrets.h`.
2. Set Wi-Fi, Signal K server, stable gateway ID, OTA, and web credentials.
3. Use different strong passwords for OTA and the web interface.

`include/secrets.h` is excluded by `.gitignore`. Never commit it. If credentials
have ever been committed elsewhere, rotate them before deploying the device.

If `WEB_ADMIN_USERNAME` or `WEB_ADMIN_PASSWORD` is absent from an older local
configuration, the firmware temporarily falls back to `admin` and the OTA
password. Set separate credentials before installation.

## Build and first flash

```sh
pio run -e az_delivery_devkit_v4_foundation
pio run -e az_delivery_devkit_v4_foundation -t upload
pio device monitor
```

After the initial USB flash, the status page is available through the DHCP
hostname or reserved IP address. Install Signal K Server 2.31 or newer and
enable a compatible consumer plugin before expecting decoded values.

## OTA

The OTA environment reads its target and password from the shell:

```sh
GATEWAY_OTA_HOST=192.0.2.31 \
GATEWAY_OTA_PASSWORD='...' \
pio run -e az_delivery_devkit_v4_foundation_ota -t upload
```

Do not put secrets in `platformio.ini`.

## Expected diagnostics

After Wi-Fi and Signal K connect, the serial log reports a running BLE scanner.
Accepted batches appear as `Delivered batch`. The status page reports received,
delivered, pending, and dropped advertisement counters, together with reset and
heap information.

## Acceptance criteria

- reproducible build and USB flash,
- stable DHCP lease and reserved-address reachability,
- authenticated SenseESP web interface,
- visible firmware version, uptime, reset reason, and heap values,
- password-protected OTA,
- serial heartbeat every 30 seconds,
- continuously running passive BLE scan,
- idempotent batch delivery to the gateway provider,
- multi-day operation without USB or unexpected resets.
